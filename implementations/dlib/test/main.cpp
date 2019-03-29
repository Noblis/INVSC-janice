#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <janice.h>

#include <chrono>

int main(int argc, char* argv[])
{
    char* sdk_path = getenv("JANICE_TEST_SDK_PATH");
    if (sdk_path == nullptr) {
        std::cerr << "No SDK path specified. Set environment variable JANICE_TEST_SDK_PATH" << std::endl;
        return 1;    
    }

    {
        JaniceError err = janice_initialize(sdk_path, "", "", "", 1, nullptr, 0);
        if (err != JANICE_SUCCESS) {
            std::cerr << "JanICE initialization failed. Error: [" << janice_error_to_string(err) << "]" << std::endl;
            return 1;
        }
    }

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    
    auto start = std::chrono::steady_clock::now();

    int result = context.run();

    auto end = std::chrono::steady_clock::now();

    auto secs   = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Tests took: " << secs << "." << millis << " seconds" << std::endl;

    {
        JaniceError err = janice_finalize();
        if (err != JANICE_SUCCESS) {
            std::cerr << "JanICE finalization failed. Error: [" << janice_error_to_string(err) << "]" << std::endl;
        }
    }

    if (context.shouldExit()) {
        return result;
    }

    return result;
}
