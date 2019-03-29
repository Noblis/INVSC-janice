#ifndef JANICE_DLIB_UTILS_HPP_INCLUDED
#define JANICE_DLIB_UTILS_HPP_INCLUDED

#define CATCH_AND_LOG(ERROR_CODE, MSG)                        \
catch (std::exception& e) {                                   \
    std::cerr << MSG << ". Error: " << e.what() << std::endl; \
    return ERROR_CODE;                                        \
} catch (...) {                                               \
    std::cerr << MSG << ". Unknown error." << std::endl;      \
    return ERROR_CODE;                                        \
}

namespace janice_dlib
{

inline bool file_exists(const std::string& filename)
{
    return std::ifstream(filename).good();
}

} // namespace janice_dlib

#endif // JANICE_DLIB_UTILS_HPP_INCLUDED
