#pragma once

#include <uv.h>

#include <iostream>
#include <thread>
#include <unordered_map>

#include "piconaut/macro.h"

// cppcheck-suppress unknownMacro
PICONAUT_INNER_NAMESPACE(sys)

enum class SignalType {
  SIGINT_SIGNAL = SIGINT,    // Interrupt signal
  SIGTERM_SIGNAL = SIGTERM,  // Termination signal
  // Hangup detected on controlling terminal or death
  // of controlling process
  SIGHUP_SIGNAL = SIGHUP,
  SIGQUIT_SIGNAL = SIGQUIT,  // Quit signal
  SIGUSR1_SIGNAL = SIGUSR1,  // User-defined signal 1
  SIGUSR2_SIGNAL = SIGUSR2,  // User-defined signal 2
  SIGABRT_SIGNAL = SIGABRT,  // Abort signal
  SIGFPE_SIGNAL = SIGFPE,    // Floating-point exception
  SIGILL_SIGNAL = SIGILL,    // Illegal instruction
  SIGSEGV_SIGNAL = SIGSEGV   // Segmentation fault
};

class SignalHandler {
 public:
  using SignalCallbackFn = void (*)(SignalHandler*, SignalType);

  SignalHandler() : loop_(uv_loop_new()), callback_(nullptr), is_run_(false) {
    InitializeDefaultSignals();
  }

  ~SignalHandler() {
    StopAllSignals();
    // uv_run(loop_, UV_RUN_NOWAIT);  // Ensure all close callbacks are processed
    // uv_loop_close(loop_);
    // uv_loop_delete(loop_);
  }

  void Start() {
    if (is_run_)
      return;

    is_run_ = true;
    thread_ = std::thread(&SignalHandler::Run, this);
  }

  void Stop() {
    StopAllSignals();
  }

  void Join() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void RegisterCallback(SignalCallbackFn callback) {
    if (callback == nullptr) {
      std::cerr << "Callback is null." << std::endl;
      return;
    }
    callback_ = callback;
  }

  void SetSignalCaptureMode(std::initializer_list<SignalType> signals) {
    for (SignalType signum : signals) {
      InitializeSignal(signum);
      StartSignal(signum);
    }
  }

 private:
  void InitializeDefaultSignals() {
    // Initialize default signals: SIGINT and SIGTERM
    InitializeSignal(SignalType::SIGINT_SIGNAL);
    InitializeSignal(SignalType::SIGTERM_SIGNAL);
    StartSignal(SignalType::SIGINT_SIGNAL);
    StartSignal(SignalType::SIGTERM_SIGNAL);
  }

  void InitializeSignal(SignalType signum) {
    int res = 0;
    uv_signal_t* handle = GetSignalHandle(signum);
    if (handle == nullptr) {
      std::cerr << "Unsupported signal: " << static_cast<int>(signum)
                << std::endl;
      return;
    }

    res = uv_signal_init(loop_, handle);
    if (res != 0) {
      std::cerr << "Failed to initialize signal handler for signal: "
                << static_cast<int>(signum) << std::endl;
    } else {
      handle->data = this;  // Set the data to the current instance
    }
  }

  void StopAllSignals() {
    for (auto& [signum, handle] : signals_) {
      uv_signal_stop(handle);
      uv_close(reinterpret_cast<uv_handle_t*>(handle), nullptr);
    }
  }

  void StartSignal(SignalType signum) {
    int res = 0;
    uv_signal_t* handle = GetSignalHandle(signum);
    if (handle == nullptr) {
      std::cerr << "Unsupported signal: " << static_cast<int>(signum)
                << std::endl;
      return;
    }
    res = uv_signal_start(handle, SignalHandler::OnSignal, static_cast<int>(signum));
    if (res != 0) {
      std::cerr << "Failed to start signal handler for signal: " << static_cast<int>(signum) << std::endl;
    }
  }

  uv_signal_t* GetSignalHandle(SignalType signum) {
    switch (signum) {
      case SignalType::SIGINT_SIGNAL:
        return &sigint_;
      case SignalType::SIGTERM_SIGNAL:
        return &sigterm_;
      case SignalType::SIGHUP_SIGNAL:
        return &sighup_;
      case SignalType::SIGQUIT_SIGNAL:
        return &sigquit_;
      case SignalType::SIGUSR1_SIGNAL:
        return &sigusr1_;
      case SignalType::SIGUSR2_SIGNAL:
        return &sigusr2_;
      case SignalType::SIGABRT_SIGNAL:
        return &sigabrt_;
      case SignalType::SIGFPE_SIGNAL:
        return &sigfpe_;
      case SignalType::SIGILL_SIGNAL:
        return &sigill_;
      case SignalType::SIGSEGV_SIGNAL:
        return &sigsegv_;
      default:
        return nullptr;
    }
  }

  static void OnSignal(uv_signal_t* handle, int signum) {
    SignalType signal = static_cast<SignalType>(signum);
    SignalHandler* handler = static_cast<SignalHandler*>(handle->data);
    if (handler && handler->callback_) {
      handler->callback_(handler, signal);
    } else {
      std::cerr << "No callback registered for signal: " << signum << std::endl;
    }

    uv_signal_stop(handle);
    uv_stop(handle->loop);
  }

  void Run() {
    std::cout << "Signal monitor run..." << std::endl;
    if (uv_run(loop_, UV_RUN_DEFAULT) != 0) {
      std::cerr << "Signal monitor failed to run." << std::endl;
    }
    std::cout << "Signal monitor stopped." << std::endl;
  }

  uv_loop_t* loop_;
  uv_signal_t sigint_;  // SIGINT (2): Interrupt signal, typically from Ctrl+C.
  uv_signal_t sigterm_; // SIGTERM (15): Termination signal for graceful shutdown.
  uv_signal_t sighup_;  // SIGHUP (1): Hangup detected on controlling terminal
                        // or death of controlling process. Often used to
                        // reload configuration.
  uv_signal_t sigquit_; // SIGQUIT (3): Quit signal, typically from Ctrl+\.
  uv_signal_t sigusr1_; // SIGUSR1 (10): User-defined signal 1.
  uv_signal_t sigusr2_; // SIGUSR2 (12): User-defined signal 2.
  uv_signal_t sigabrt_; // SIGABRT (6): Abort signal, usually initiated by
                        // the abort function.
  uv_signal_t sigfpe_;  // SIGFPE (8): Floating-point exception.
  uv_signal_t sigill_;  // SIGILL (4): Illegal instruction.
  uv_signal_t sigsegv_; // SIGSEGV (11): Segmentation fault.
  std::thread thread_;
  SignalCallbackFn callback_;
  std::unordered_map<SignalType, uv_signal_t*> signals_;
  bool is_run_;
};

PICONAUT_INNER_END_NAMESPACE
