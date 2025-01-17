#define __APICALL_EXTERN
#include "dmem.h"
#undef __APICALL_EXTERN

#include "core/imports/imports_gpuMemory.h"
#include "core/memory/memory.h"
#include "logging.h"
#include "utility/utility.h"

#include <algorithm>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <mutex>
#include <vector>

LOG_DEFINE_MODULE(MemoryManager);

namespace {

static uint64_t getAligned(uint64_t pos, size_t align) {
  return (align != 0 ? (pos + (align - 1)) & ~(align - 1) : pos);
}
} // namespace

// Allocates the sections (memory pool is done internally (in app))
class PhysicalMemory: public IPysicalMemory {
  std::mutex m_mutex_int;

  public:
  PhysicalMemory() = default;
  uint64_t  alloc(uint64_t vaddr, size_t len, int memoryTye) final;
  bool      reserve(uint64_t start, size_t len, size_t alignment, uint64_t* outAddr, int memoryType) final;
  uintptr_t commit(uint64_t base, uint64_t offset, size_t len, size_t alignment, int prot) final;
  bool      Map(uint64_t vaddr, uint64_t physAddr, size_t len, int prot, bool allocFixed, size_t alignment, uint64_t* outAddr) final;
  bool      Release(uint64_t start, size_t len, uint64_t* vaddr, uint64_t* size) final;
  bool      Unmap(uint64_t vaddr, uint64_t size) final;
};

IPysicalMemory& accessPysicalMemory() {
  static PhysicalMemory inst;
  return inst;
}

uint64_t PhysicalMemory::alloc(uint64_t vaddr, size_t len, int prot) {
  LOG_USE_MODULE(MemoryManager);
  m_allocSize += len;
  LOG_DEBUG(L"Alloc: 0x%08llx len:0x%08llx prot:%d curSize=0x%08llx", vaddr, len, prot, m_allocSize);
  return 0;
}

bool PhysicalMemory::reserve(uint64_t start, size_t len, size_t alignment, uint64_t* outAddr, int memoryType) {
  LOG_USE_MODULE(MemoryManager);

  auto const isGpu = memoryType == 3;
  *outAddr         = memory::reserve(start, len, alignment, isGpu);

  {
    std::unique_lock const lock(m_mutex_int);
    m_availableSize -= len;
  }

  LOG_DEBUG(L"Reserve| start:0x%08llx size:%llu alignment:%llu memType:%d -> @%08llx", start, len, alignment, memoryType, *outAddr);
  return *outAddr != 0;
}

uintptr_t PhysicalMemory::commit(uint64_t base, uint64_t vaddr, size_t len, size_t alignment, int prot) {
  LOG_USE_MODULE(MemoryManager);

  uintptr_t  addr  = 0;
  auto const isGpu = (prot & 0xF0) > 0;
  if (isGpu & ((prot & 0xF) > 0)) {
    addr = memory::allocGPUMemory(base, 0, len, alignment);
  } else {
    addr = memory::commit(base, 0, len, alignment, prot);
  }

  if (isGpu) {
    if (!gpuMemory::notify_allocHeap(addr, len, prot)) {
      LOG_ERR(L"Commit| Couldn't allocHeap| base:0x%08llx offset:0x%08llx size:%llu alignment:%llu prot:%d -> @%08llx", base, vaddr, len, alignment, prot,
              addr);
      return 0;
    }
  }
  LOG_DEBUG(L"Commit| base:0x%08llx offset:0x%08llx size:%llu alignment:%llu prot:%d -> @%08llx", base, vaddr, len, alignment, prot, addr);
  return addr;
}

bool PhysicalMemory::Map(uint64_t vaddr, uint64_t physAddr, size_t len, int prot, bool allocFixed, size_t alignment, uint64_t* outAddr) {
  LOG_USE_MODULE(MemoryManager);

  bool mapped = false;
  *outAddr    = 0;
  {
    std::unique_lock const lock(m_mutex_int);

    auto [protCPU, protGPU] = util::getMemoryProtection(prot);

    if (allocFixed) {
      if (memory::allocFixed(physAddr, len, prot)) {
        *outAddr = physAddr;
      }
    } else {
      *outAddr = memory::allocAligned(physAddr, len, prot, alignment);
    }

    if (protGPU != 0) {
      if (!gpuMemory::notify_allocHeap(*outAddr, len, prot)) {
        LOG_ERR(L"Map| Couldn't allocHeap vaddr:0x%08llx physAddr:0x%08llx len:0x%08llx prot:0x%x -> out:0x%08llx", vaddr, physAddr, len, prot, *outAddr);
        return false;
      }
    }

    if (*outAddr == NULL) {
      return false;
    }
    m_availableSize -= len;
  }

  LOG_INFO(L"Map| vaddr:0x%08llx physAddr:0x%08llx len:0x%08llx prot:0x%x -> out:0x%08llx", vaddr, physAddr, len, prot, *outAddr);
  return true;
}

bool PhysicalMemory::Release(uint64_t start, size_t len, uint64_t* vaddr, uint64_t* size) {
  LOG_USE_MODULE(MemoryManager);
  LOG_ERR(L"todo %S", __FUNCTION__);
  m_allocSize -= len;
  return true;
}

bool PhysicalMemory::Unmap(uint64_t vaddr, uint64_t size) {
  LOG_USE_MODULE(MemoryManager);

  memory::free(vaddr);
  {
    std::unique_lock const lock(m_mutex_int);
    m_availableSize += size;
  }
  // if(isGPU) accessGpuMemory().freeHeap(vaddr); // todo

  LOG_INFO(L"Unmap: vaddr:0x%08llx len:%lld", vaddr, size);
  return true;
}

class FlexibleMemory: public IFlexibleMemory {
  uint64_t   m_totalAllocated = 0;
  std::mutex m_mutex_int;

  public:
  FlexibleMemory() = default;

  uint64_t alloc(uint64_t vaddr, size_t len, int prot) final;
  bool     destroy(uint64_t vaddr, uint64_t size) final;

  void release(uint64_t start, size_t len) final;

  uint64_t available() final {
    std::unique_lock const lock(m_mutex_int);
    return (uint64_t)448 * 1024 * 1024 - m_totalAllocated; // todo get system ram
  }
};

IFlexibleMemory& accessFlexibleMemory() {
  static FlexibleMemory inst;
  return inst;
}

uint64_t FlexibleMemory::alloc(uint64_t vaddr, size_t len, int prot) {
  LOG_USE_MODULE(MemoryManager);

  std::unique_lock const lock(m_mutex_int);
  m_totalAllocated += len;

  auto const outAddr = memory::alloc(vaddr, len, prot);
  LOG_INFO(L"--> Heap| vaddr:0x%08llx len:%llu prot:0x%x total:0x%08llx -> @0x%08llx", vaddr, len, prot, m_totalAllocated, outAddr);
  return outAddr;
}

bool FlexibleMemory::destroy(uint64_t vaddr, uint64_t size) {
  LOG_USE_MODULE(MemoryManager);

  std::unique_lock const lock(m_mutex_int);
  m_totalAllocated -= size;

  memory::free(vaddr);
  LOG_INFO(L"<-- Heap| vaddr:0x%08llx len:%lld total:0x%08llx", vaddr, size, m_totalAllocated);

  return true;
}

void FlexibleMemory::release(uint64_t start, size_t len) {
  LOG_USE_MODULE(MemoryManager);
  LOG_ERR(L"todo %S", __FUNCTION__);
}