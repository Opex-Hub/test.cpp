#include <jni.h>
#include <android/log.h>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <chrono>
#include <thread>

// Enable NEON vectorization for ARM64
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#define LOG_TAG "EigenAndroidWrapper"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace Eigen;

// Optimized matrix multiplication using Eigen with ARM64 optimizations
extern "C" JNIEXPORT jdoubleArray JNICALL
Java_com_example_eigenandroid_EigenWrapper_multiplyMatrices(
        JNIEnv *env,
        jobject /* this */,
        jdoubleArray java_a,
        jdoubleArray java_b,
        jint rows_a,
        jint cols_a,
        jint cols_b) {

    // Get input arrays
    jdouble *a_data = env->GetDoubleArrayElements(java_a, nullptr);
    jdouble *b_data = env->GetDoubleArrayElements(java_b, nullptr);

    // Create Eigen maps for zero-copy access
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> 
        a_map(a_data, rows_a, cols_a);
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> 
        b_map(b_data, cols_a, cols_b);

    // Allocate result matrix
    MatrixXd result = MatrixXd::Zero(rows_a, cols_b);

    // Perform optimized matrix multiplication
    // Using Eigen's built-in optimization with compile-time settings
    const int SIMD_SIZE = 4; // ARM64 NEON processes 4 doubles per instruction
    
    // Parallelize computation based on mobile cores
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < rows_a; i += SIMD_SIZE) {
        for (int j = 0; j < cols_b; j += SIMD_SIZE) {
            // Calculate block dimensions
            int block_rows = std::min(SIMD_SIZE, rows_a - i);
            int block_cols = std::min(SIMD_SIZE, cols_b - j);
            
            // Perform block multiplication
            result.block(i, j, block_rows, block_cols) =
                a_map.block(i, 0, block_rows, cols_a) *
                b_map.block(0, j, cols_a, block_cols);
        }
    }

    // Release arrays
    env->ReleaseDoubleArrayElements(java_a, a_data, JNI_ABORT);
    env->ReleaseDoubleArrayElements(java_b, b_data, JNI_ABORT);

    // Return result
    jdoubleArray result_array = env->NewDoubleArray(rows_a * cols_b);
    env->SetDoubleArrayRegion(result_array, 0, rows_a * cols_b, result.data());
    
    return result_array;
}

// Optimized vector transformation using Eigen
extern "C" JNIEXPORT jdoubleArray JNICALL
Java_com_example_eigenandroid_EigenWrapper_transformVector(
        JNIEnv *env,
        jobject /* this */,
        jdoubleArray java_matrix,
        jdoubleArray java_vector,
        jint rows,
        jint cols) {

    // Get input arrays
    jdouble *matrix_data = env->GetDoubleArrayElements(java_matrix, nullptr);
    jdouble *vector_data = env->GetDoubleArrayElements(java_vector, nullptr);

    // Create Eigen maps
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> 
        matrix_map(matrix_data, rows, cols);
    Map<VectorXd> vector_map(vector_data, cols);

    // Perform transformation
    VectorXd result = matrix_map * vector_map;

    // Release arrays
    env->ReleaseDoubleArrayElements(java_matrix, matrix_data, JNI_ABORT);
    env->ReleaseDoubleArrayElements(java_vector, vector_data, JNI_ABORT);

    // Return result
    jdoubleArray result_array = env->NewDoubleArray(rows);
    env->SetDoubleArrayRegion(result_array, 0, rows, result.data());
    
    return result_array;
}

// Compute eigenvalues and eigenvectors for symmetric matrices
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_example_eigenandroid_EigenWrapper_computeEigenDecomposition(
        JNIEnv *env,
        jobject /* this */,
        jdoubleArray java_matrix,
        jint size) {

    // Get input array
    jdouble *matrix_data = env->GetDoubleArrayElements(java_matrix, nullptr);

    // Create Eigen map for symmetric matrix
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> 
        matrix_map(matrix_data, size, size);

    // Ensure matrix is symmetric (needed for SelfAdjointEigenSolver)
    MatrixXd sym_matrix = (matrix_map + matrix_map.transpose()) / 2.0;

    // Solve eigenvalue problem
    SelfAdjointEigenSolver<MatrixXd> solver(sym_matrix);

    if (solver.info() != Success) {
        LOGE("Eigen decomposition failed");
        env->ReleaseDoubleArrayElements(java_matrix, matrix_data, JNI_ABORT);
        return nullptr;
    }

    // Get results
    VectorXd eigenvalues = solver.eigenvalues();
    MatrixXd eigenvectors = solver.eigenvectors();

    // Release input array
    env->ReleaseDoubleArrayElements(java_matrix, matrix_data, JNI_ABORT);

    // Prepare output
    jclass double_array_class = env->FindClass("[D");
    jobjectArray result = env->NewObjectArray(2, double_array_class, nullptr);
    
    // Set eigenvalues
    jdoubleArray eigenvalues_array = env->NewDoubleArray(size);
    env->SetDoubleArrayRegion(eigenvalues_array, 0, size, eigenvalues.data());
    env->SetObjectArrayElement(result, 0, eigenvalues_array);
    
    // Set eigenvectors (flatten matrix)
    jdoubleArray eigenvectors_array = env->NewDoubleArray(size * size);
    env->SetDoubleArrayRegion(eigenvectors_array, 0, size * size, eigenvectors.data());
    env->SetObjectArrayElement(result, 1, eigenvectors_array);

    return result;
}

// Perform singular value decomposition
extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_example_eigenandroid_EigenWrapper_computeSVD(
        JNIEnv *env,
        jobject /* this */,
        jdoubleArray java_matrix,
        jint rows,
        jint cols) {

    // Get input array
    jdouble *matrix_data = env->GetDoubleArrayElements(java_matrix, nullptr);

    // Create Eigen map
    Map<Matrix<double, Dynamic, Dynamic, RowMajor>> 
        matrix_map(matrix_data, rows, cols);

    // Compute SVD
    JacobiSVD<MatrixXd> svd(matrix_map, ComputeThinU | ComputeThinV);

    // Get results
    VectorXd singular_values = svd.singularValues();
    MatrixXd u = svd.matrixU();
    MatrixXd v = svd.matrixV();

    // Release input array
    env->ReleaseDoubleArrayElements(java_matrix, matrix_data, JNI_ABORT);

    // Prepare output
    jclass double_array_class = env->FindClass("[D");
    jobjectArray result = env->NewObjectArray(3, double_array_class, nullptr);
    
    // Set singular values
    jdoubleArray s_array = env->NewDoubleArray(singular_values.size());
    env->SetDoubleArrayRegion(s_array, 0, singular_values.size(), singular_values.data());
    env->SetObjectArrayElement(result, 0, s_array);
    
    // Set U matrix
    jdoubleArray u_array = env->NewDoubleArray(u.rows() * u.cols());
    env->SetDoubleArrayRegion(u_array, 0, u.rows() * u.cols(), u.data());
    env->SetObjectArrayElement(result, 1, u_array);
    
    // Set V matrix
    jdoubleArray v_array = env->NewDoubleArray(v.rows() * v.cols());
    env->SetDoubleArrayRegion(v_array, 0, v.rows() * v.cols(), v.data());
    env->SetObjectArrayElement(result, 2, v_array);

    return result;
}

// Initialize Eigen threading for mobile
extern "C" JNIEXPORT void JNICALL
Java_com_example_eigenandroid_EigenWrapper_initializeEigen(
        JNIEnv */* env */,
        jobject /* this */) {
        
    // Set number of threads based on available CPU cores
    // Most modern Android devices have 4-8 cores
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads > 8) num_threads = 8; // Limit for mobile
    
    #ifdef EIGEN_DONT_PARALLELIZE
    LOGI("Eigen parallelization disabled");
    #else
    Eigen::setNbThreads(num_threads);
    LOGI("Eigen initialized with %d threads", num_threads);
    #endif
    
    // Log NEON support status
    #ifdef __ARM_NEON
    LOGI("NEON optimizations enabled");
    #else
    LOGI("NEON optimizations not available");
    #endif
}
