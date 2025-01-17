#include "dmem.h"

#include "assert.h"
#include "common.h"
#include "core/dmem/dmem.h"
#include "core/imports/imports_gpuMemory.h"
#include "core/memory/memory.h"
#include "logging.h"
#include "modules_include/common.h"
#include "types.h"

LOG_DEFINE_MODULE(dmem)

namespace {} // namespace

extern "C" {

EXPORT SYSV_ABI size_t sceKernelGetDirectMemorySize() {
  LOG_USE_MODULE(dmem);

  auto size = accessPysicalMemory().size();
  LOG_DEBUG(L"%S size:0x%08llx", __FUNCTION__, size);
  return size;
}

EXPORT SYSV_ABI int32_t sceKernelMapNamedFlexibleMemory(void** addrInOut, size_t len, int prot, int flags, const char* name) {
  auto const inAddr  = reinterpret_cast<uint64_t>(*addrInOut);
  auto const outAddr = accessFlexibleMemory().alloc(inAddr, len, prot);
  *addrInOut         = reinterpret_cast<void*>(outAddr);

  if (outAddr == 0) {
    return getErr(ErrCode::_ENOMEM);
  }

  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMapFlexibleMemory(void** addrInOut, size_t len, int prot, int flags) {
  return sceKernelMapNamedFlexibleMemory(addrInOut, len, prot, flags, "");
}

EXPORT SYSV_ABI int32_t sceKernelReleaseFlexibleMemory(void* addr, size_t len) {
  accessFlexibleMemory().release((uint64_t)addr, len);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelSetPrtAperture(int index, void* addr, size_t len) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelGetPrtAperture(int index, void** addr, size_t* len) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelAllocateDirectMemory(off_t searchStart, off_t searchEnd, size_t len, size_t alignment, int memoryType, off_t* physAddrOut) {
  if (len == 0 || physAddrOut == nullptr) {
    return getErr(ErrCode::_EINVAL);
  }

  accessPysicalMemory().alloc(0, len, memoryType);
  *physAddrOut = 1; // done with map
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelAllocateMainDirectMemory(size_t len, size_t alignment, int memoryType, off_t* physAddrOut) {
  if (len == 0 || physAddrOut == nullptr) {
    return getErr(ErrCode::_EINVAL);
  }

  *physAddrOut = 1; // done with map
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelReleaseDirectMemory(off_t start, size_t len) {
  uint64_t vaddr = 0;
  uint64_t size  = 0;

  bool result = accessPysicalMemory().Release(start, len, &vaddr, &size);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelCheckedReleaseDirectMemory(off_t start, size_t len) {
  uint64_t vaddr = 0;
  uint64_t size  = 0;

  bool result = accessPysicalMemory().Release(start, len, &vaddr, &size);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMapNamedDirectMemory(uint64_t* addr, size_t len, int prot, int flags, off_t directMemoryStart, size_t alignment,
                                                      const char* name) {
  if (!accessPysicalMemory().Map(*addr, directMemoryStart, len, prot, flags == 0x10, alignment, addr)) {
    return getErr(ErrCode::_EINVAL);
  }

  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMapDirectMemory(uint64_t* addr, size_t len, int prot, int flags, off_t directMemoryStart, size_t maxPageSize) {
  return sceKernelMapNamedDirectMemory(addr, len, prot, flags, directMemoryStart, maxPageSize, nullptr);
}

EXPORT SYSV_ABI int32_t sceKernelMapDirectMemory2(uint64_t* addr, size_t len, int type, int prot, int flags, off_t directMemoryStart, size_t maxPageSize) {
  return sceKernelMapNamedDirectMemory(addr, len, prot, flags, directMemoryStart, maxPageSize, nullptr);
}

EXPORT SYSV_ABI int32_t sceKernelGetDirectMemoryType(off_t start, int* memoryType, off_t* regionStartOut, off_t* regionEndOut) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelBatchMap(SceKernelBatchMapEntry* items, int size, int* count) {
  struct BatchMapEntry {
    uint64_t start;
    uint32_t physAddr;
    size_t   length;
    uint8_t  prot;
    uint8_t  type;
    short    pad1;
    int      operation;
  };

  for (*count = 0; *count < size; ++*count) {
    auto& batchEntry = ((BatchMapEntry*)items)[*count];

    uint64_t addr = accessPysicalMemory().commit(batchEntry.start, batchEntry.physAddr, batchEntry.length, 0, batchEntry.prot);
    if (addr == 0) {
      return getErr(ErrCode::_ENOMEM);
    }
  }
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelBatchMap2(SceKernelBatchMapEntry* entries, int numberOfEntries, int* numberOfEntriesOut, int flags) {
  return sceKernelBatchMap(entries, numberOfEntries, numberOfEntriesOut);
}

EXPORT SYSV_ABI int32_t sceKernelJitCreateSharedMemory(const char* name, size_t len, int maxProt, int* fdOut) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelJitCreateAliasOfSharedMemory(int fd, int maxProt, int* fdOut) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelJitMapSharedMemory(int fd, int prot, void** startOut) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelJitGetSharedMemoryInfo(int fd, char* name, int nameBufferSize, void** startOut, size_t* lenOut, int* maxProtOut) {
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelQueryMemoryProtection(uintptr_t addr, uintptr_t* start, uintptr_t* end, int* prot) {
  LOG_USE_MODULE(dmem);

  uint64_t base = memory::queryAlloc(addr, start, end, prot);
  if (base == 0) {
    LOG_TRACE(L"query KERNEL_ERROR_EACCES: 0x%08llx", (uint64_t)addr);
    return getErr(ErrCode::_EACCES);
  }

  if (prot != nullptr) {
    if (gpuMemory::isGPULocal(base)) *prot |= 0x30; // GPU read write
    LOG_TRACE(L"query: 0x%08llx prot:%x", (uint64_t)addr, *prot);
  }

  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelIsStack(void* addr, void** start, void** end) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelVirtualQuery(const void* addr, int flags, SceKernelVirtualQueryInfo* info, size_t infoSize) {
  LOG_USE_MODULE(dmem);

  uint64_t base = (uint64_t)addr;

  if ((uint64_t)addr < IMAGE_BASE) {
    info->start      = 0;
    info->end        = (void*)IMAGE_BASE;
    info->protection = SCE_KERNEL_PROT_CPU_ALL;
  } else {
    base = memory::queryAlloc((uintptr_t)addr, (uintptr_t*)&info->start, (uintptr_t*)&info->end, &info->protection);
    if (base <= 0) {
      LOG_TRACE(L"Query Error: addr:0x%08llx, flag:%d, infoSize:%llu - start:0x%08llx, end:0x%08llx, type:%d", addr, flags, infoSize, info->start, info->end,
                info->memoryType);
      return getErr(ErrCode::_EACCES);
    }
  }

  if (gpuMemory::isGPULocal(base)) {
    info->memoryType     = 3;
    info->isDirectMemory = true;
    info->protection |= 0x30; // GPU read write
  } else {
    info->isDirectMemory = false;
  }

  info->isCommitted      = true;
  info->isFlexibleMemory = true;
  info->offset           = (int64_t)addr - (int64_t)info->start;
  LOG_TRACE(L"Query OK: addr:0x%08llx, flag:%d, infoSize:%llu - start:0x%08llx end:0x%08llx prot:%d type:%d", addr, flags, infoSize, info->start, info->end,
            info->protection, info->memoryType);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelSetVirtualRangeName(void* start, size_t len, const char* name) {
  LOG_USE_MODULE(dmem);
  LOG_TRACE(L"vrange, start:0x%08llx len:0x%08llx name:%S", start, len, name);
  return Ok;
}

EXPORT SYSV_ABI int sceKernelReserveVirtualRange(uintptr_t* addr, size_t len, int flags, size_t alignment) {
  auto const inAddr = *addr;

  if (len == 0) {
    return getErr(ErrCode::_EINVAL);
  }

  if (!accessPysicalMemory().reserve(inAddr, len, alignment, addr, 1)) {
    return getErr(ErrCode::_EAGAIN);
  }

  return Ok;
}

struct QueryInfo {
  uintptr_t start;
  uintptr_t end;
  int       memoryType;
};

EXPORT SYSV_ABI int32_t sceKernelDirectMemoryQuery(uint64_t offset, int flags, QueryInfo* query_info, size_t infoSize) {

  LOG_USE_MODULE(dmem);

  if (offset < 0 || infoSize != sizeof(QueryInfo) || query_info == nullptr) {
    return getErr(ErrCode::_EINVAL);
  }

  offset = std::max(offset, DIRECTMEM_START);

  int prot;

  int64_t base = memory::queryAlloc(offset, &query_info->start, &query_info->end, &prot);
  if (base <= 0) {
    LOG_TRACE(L"Query Error: offset:0x%08llx, flag:%d, infoSize:%llu - start:0x%08llx, end:0x%08llx, type:%d", offset, flags, infoSize, query_info->start,
              query_info->end, query_info->memoryType);
    return getErr(ErrCode::_EACCES);
  }

  query_info->memoryType = gpuMemory::isGPULocal(base) ? 3 : 0;

  LOG_TRACE(L"Query Ok: offset:0x%08llx, flag:%d, infoSize:%llu - start:0x%08llx, end:0x%08llx, type:%d", offset, flags, infoSize, query_info->start,
            query_info->end, query_info->memoryType);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMtypeprotect(const void* addr, size_t size, int type, int prot) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelAvailableDirectMemorySize(off_t start, off_t end, size_t alignment, uint32_t* startOut, size_t* sizeOut) {
  accessPysicalMemory().getAvailableSize(start, end, alignment, startOut, sizeOut);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelGetPageTableStats(int* cpuTotal, int* cpuAvailable, int* gpuTotal, int* gpuAvailable) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolBatch(const SceKernelMemoryPoolBatchEntry* entries, int n, int* indexOut, int flags) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolCommit(uint64_t addr, size_t len, int type, int prot, int flags) {
  LOG_USE_MODULE(dmem);

  uint64_t   outAddr = 0;
  auto const isGpu   = (prot & 0xF0) > 0;
  if (isGpu & ((prot & 0xF) > 0)) {
    outAddr = memory::allocGPUMemory(addr, 0, len, 0);
  } else {
    outAddr = memory::commit(addr, 0, len, 0, prot);
  }
  assert(addr == outAddr);
  LOG_DEBUG(L"poolCommit| addr:0x%08llx len:0x%08llx type:%d prot:%d flag:%d @0x%08llx", addr, len, type, prot, flags, outAddr);

  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolDecommit(void* addr, size_t len, int flags) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolExpand(off_t searchStart, off_t searchEnd, size_t len, size_t alignment, off_t* physAddrOut) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S| start:0x%08llx end:0x%08llx, len:0x%08llx  align:0x%08llx", __FUNCTION__, searchStart, searchEnd, len, alignment);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolGetBlockStats(SceKernelMemoryPoolBlockStats* output, size_t outputSize) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolMove(void* dst, void* src, size_t len, int flags) {
  LOG_USE_MODULE(dmem);
  LOG_ERR(L"todo %S", __FUNCTION__);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelMemoryPoolReserve(uint64_t addrIn, uint64_t len, uint64_t alignment, int flags, uint64_t* addrOut) {
  LOG_USE_MODULE(dmem);

  *addrOut = memory::reserve(addrIn, len, alignment, false);
  LOG_DEBUG(L"PoolReserve| start:0x%08llx, len:0x%08llx  align:0x%08llx flags:%d -> out:0x%08llx", addrIn, len, alignment, flags, *addrOut);
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelAvailableFlexibleMemorySize(size_t* sizeOut) {
  *sizeOut = accessFlexibleMemory().available();
  return Ok;
}

EXPORT SYSV_ABI int32_t sceKernelConfiguredFlexibleMemorySize(size_t* sizeOut) {
  *sizeOut = accessFlexibleMemory().size();
  return Ok;
}

} // namespace dmem