#pragma once
#include <stdint.h>

namespace Err {
constexpr int VIDEO_OUT_ERROR_INVALID_VALUE                    = -2144796671; /* 0x80290001 */
constexpr int VIDEO_OUT_ERROR_INVALID_ADDRESS                  = -2144796670; /* 0x80290002 */
constexpr int VIDEO_OUT_ERROR_INVALID_PIXEL_FORMAT             = -2144796669; /* 0x80290003 */
constexpr int VIDEO_OUT_ERROR_INVALID_PITCH                    = -2144796668; /* 0x80290004 */
constexpr int VIDEO_OUT_ERROR_INVALID_RESOLUTION               = -2144796667; /* 0x80290005 */
constexpr int VIDEO_OUT_ERROR_INVALID_FLIP_MODE                = -2144796666; /* 0x80290006 */
constexpr int VIDEO_OUT_ERROR_INVALID_TILING_MODE              = -2144796665; /* 0x80290007 */
constexpr int VIDEO_OUT_ERROR_INVALID_ASPECT_RATIO             = -2144796664; /* 0x80290008 */
constexpr int VIDEO_OUT_ERROR_RESOURCE_BUSY                    = -2144796663; /* 0x80290009 */
constexpr int VIDEO_OUT_ERROR_INVALID_INDEX                    = -2144796662; /* 0x8029000A */
constexpr int VIDEO_OUT_ERROR_INVALID_HANDLE                   = -2144796661; /* 0x8029000B */
constexpr int VIDEO_OUT_ERROR_INVALID_EVENT_QUEUE              = -2144796660; /* 0x8029000C */
constexpr int VIDEO_OUT_ERROR_INVALID_EVENT                    = -2144796659; /* 0x8029000D */
constexpr int VIDEO_OUT_ERROR_NO_EMPTY_SLOT                    = -2144796657; /* 0x8029000F */
constexpr int VIDEO_OUT_ERROR_SLOT_OCCUPIED                    = -2144796656; /* 0x80290010 */
constexpr int VIDEO_OUT_ERROR_FLIP_QUEUE_FULL                  = -2144796654; /* 0x80290012 */
constexpr int VIDEO_OUT_ERROR_INVALID_MEMORY                   = -2144796653; /* 0x80290013 */
constexpr int VIDEO_OUT_ERROR_MEMORY_NOT_PHYSICALLY_CONTIGUOUS = -2144796652; /* 0x80290014 */
constexpr int VIDEO_OUT_ERROR_MEMORY_INVALID_ALIGNMENT         = -2144796651; /* 0x80290015 */
constexpr int VIDEO_OUT_ERROR_UNSUPPORTED_OUTPUT_MODE          = -2144796650; /* 0x80290016 */
constexpr int VIDEO_OUT_ERROR_OVERFLOW                         = -2144796649; /* 0x80290017 */
constexpr int VIDEO_OUT_ERROR_NO_DEVICE                        = -2144796648; /* 0x80290018 */
constexpr int VIDEO_OUT_ERROR_UNAVAILABLE_OUTPUT_MODE          = -2144796647; /* 0x80290019 */
constexpr int VIDEO_OUT_ERROR_INVALID_OPTION                   = -2144796646; /* 0x8029001A */
constexpr int VIDEO_OUT_ERROR_PORT_UNSUPPORTED_FUNCTION        = -2144796645; /* 0x8029001B */
constexpr int VIDEO_OUT_ERROR_UNSUPPORTED_OPERATION            = -2144796644; /* 0x8029001C */
constexpr int VIDEO_OUT_ERROR_FATAL                            = -2144796417; /* 0x802900FF */
constexpr int VIDEO_OUT_ERROR_UNKNOWN                          = -2144796418; /* 0x802900FE */
constexpr int VIDEO_OUT_ERROR_ENOMEM                           = -2144792564; /* 0x8029100C */
} // namespace Err