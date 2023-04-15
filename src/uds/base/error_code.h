/**
 * @file error_code.h
 * @brief 错误代码.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-31
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_ERROR_CODE_H_
#define IC_UDS_BASE_ERROR_CODE_H_
#include <system_error>

namespace ic {
namespace uds {

enum class BaseErrc {
    ReInitialization = 1,
    CreateSocketFailed,
    BusySocketFile,
    BindFailed,
    NotInitialized,
    SendFailed,
    RecvFailed,
    Timeout,
}; // enum class BaseErrc

std::error_code make_error_code(BaseErrc ec);

} // namespace uds
} // namespace ic

namespace std {
template<>
struct is_error_code_enum<ic::uds::BaseErrc>: true_type {};
} // namespace std

#endif // IC_UDS_BASE_ERROR_CODE_H_
