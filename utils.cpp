#include "examples.h"

void ImageShow(String str) {
    Mat image = imread(str);

    // 检查图像是否成功加载
    if (image.empty()) {
        std::cerr << "Error: Could not read the image." << std::endl;
        return;
    }
    int height = image.rows;
    int width = image.cols;
    int channel = image.channels();

    int window_height = 600;
    int window_width = static_cast<int>(window_height * ((float)width / height));
    // 创建一个可调整大小的窗口
    namedWindow("Resizable Window", WINDOW_NORMAL);

    resizeWindow("Resizable Window", window_width, window_height);

    // 显示图像
    imshow("Resizable Window", image);
    waitKey(0);
}

void mul(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>&result) {
    if (vector_1.size() != vector_2.size()) {
        std::cerr << "Error: Vectors must have the same size for addition." << std::endl;
        result = vector<double>();
    }
    for (size_t i = 0; i < vector_1.size(); i++)
    {
        double x = vector_1[i];
        double y = vector_2[i];
        result.push_back(x * y);
    }
}
double dot(const vector<double>& vector_1, const vector<double>& vector_2) {
    if (vector_1.size() != vector_2.size()) {
        std::cerr << "Error: Vectors must have the same size for addition." << std::endl;
        return 0;
    }
    vector<double> result;
    mul(vector_1, vector_2, result);
    double sum = accumulate(result.begin(), result.end(), 0.0);
    return sum;
}
void add(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>& result) {
    if (vector_1.size() != vector_2.size()) {
        std::cerr << "Error: Vectors must have the same size for addition." << std::endl;
        result = vector<double>();
    }
    for (size_t i = 0; i < vector_1.size(); i++)
    {
        double x = vector_1[i];
        double y = vector_2[i];
        result.push_back(x + y);
    }
}
double add_self(const vector<double>& vector) {
    double ret = 0.0;
    for (auto& num : vector) {
        ret += num;
    }
    return ret;
}
bool isImage(string path) {
    vector<string> types = { ".jpg", ".jpeg", ".png", ".bmp", ".gif" };
    string fileExtension = fs::path(path).extension().string();
    transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
    return find(types.begin(), types.end(), fileExtension) != types.end();
}
void getImagePath(const string &dir, vector<string>&imagePaths) {
    try {
        for (const auto& entry : directory_iterator(dir)) {
            string path = entry.path().string();
            if (isImage(path)) {
                imagePaths.push_back(path);
            }
        }
    }
    catch (const filesystem_error& e) {
        cerr << "Error: " << e.what() << endl;
    }
}
void getImageVector(const string& str, vector<vector<double>>& result) {
    if (!isImage(str)) {
        cout << "Error (not an image): " << str << endl;
        result = vector<vector<double>>();
        return;
    }
    Mat image = imread(str);
    if (image.empty()) {
        cerr << "Error: can't read image" << endl;
        result = vector<vector<double>>();
        return;
    }
    int height = image.rows;
    int width = image.cols;
    if (height * width > 4096) {
        cerr << "Error: image is too big" << endl;
        result = vector<vector<double>>();
        return;
    }
    int channel = image.channels();
    if(!result.empty())
        result.clear();  // 确保result为空

    if (channel == 1 || channel == 3 || channel == 4) {
        result.resize(channel, std::vector<double>());  // 初始化 result 的第一维
    }
    else {
        cerr << "Error: Unsupported number of channels (" << channel << ")" << endl;
        result = vector<vector<double>>();
        return;
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (channel == 1) {
                result[0].push_back(static_cast<double>(image.at<uchar>(i, j))/255);
            }
            else if (channel == 3) {
                Vec3b pixel = image.at<Vec3b>(i, j);
                for (int c = 0; c < channel; ++c) {
                    result[c].push_back(static_cast<double>(pixel[c]) / 255);
                }
            }
            else if (channel == 4) {
                Vec4b pixel = image.at<Vec4b>(i, j);
                for (int c = 0; c < channel; ++c) {
                    result[c].push_back(static_cast<double>(pixel[c]) / 255);
                }
            }
        }
    }
}
void getFilePath(const string& dir, vector<string>& Paths) {
    try {
        for (const auto& entry : directory_iterator(dir)) {
            string path = entry.path().string();
            Paths.push_back(path);
        }
    }
    catch (const filesystem_error& e) {
        cerr << "Error: " << e.what() << endl;
    }
}
void getSubDir(const string& dir, vector<string>& Paths) {
    if (is_directory(dir)) {
        try {
            for (const auto& entry : directory_iterator(dir)) {
                if (is_directory(entry.status())) {
                    string temp = entry.path().string();
                    Paths.push_back(temp);
                }
            }
        }
        catch (const fs::filesystem_error& ex) {
            std::cerr << "Error accessing directory: " << ex.what() << std::endl;
        }
    }
    else {
        cerr << "Invalid directory path." << endl;
    }
}
bool equalImage(const string& str, const vector<vector<double>>& input) {
    if (!isImage(str) || input.empty()) {
        cout << "Error (not an image): " << str << endl;
        return false;
    }
    Mat image = imread(str);
    if (image.empty()) {
        cerr << "Error: can't read image" << endl;
        return false;
    }
    int height = image.rows;
    int width = image.cols;
    int channel = image.channels();
    if (input[0].size() < height * width || channel != input.size()) {
        cout << "size is not same" << endl;
        return false;
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (channel == 1) {
                if (image.at<uchar>(i, j) != static_cast<uchar>(round(input[0][i * width + j] * 255))) {
                    return false;
                }
            }
            else if (channel == 3) {
                Vec3b pixel = image.at<Vec3b>(i, j);
                for (int c = 0; c < channel; ++c) {
                    if (pixel[c] != static_cast<uchar>(round(input[c][i * width + j] * 255))) {
                        /*ios old_fmt(nullptr);
                        old_fmt.copyfmt(cout);
                        cout << fixed << setprecision(10);
                        cout << "not same at (" << i << ", " << j << ") " << (int)pixel[c] << "  " << input[c][i * width + j] * 255 << endl;
                        cout.copyfmt(old_fmt);*/
                        return false;
                    }
                }
            }
            else if (channel == 4) {
                Vec4b pixel = image.at<Vec4b>(i, j);
                for (int c = 0; c < channel; ++c) {
                    if (pixel[c] != static_cast<uchar>(round(input[c][i * width + j] * 255))) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}
bool equalImage(const vector<vector<double>>& image1, const vector<vector<double>>& image2) {
    if (image1.size() != image2.size() || image1[0].size() != image2[0].size()) {
        return false;
    }
    int rows = (int)image1.size();
    int cols = (int)image1[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (round(image1[i][j]*255) != round(image2[i][j]*255))
                return false;
        }
    }
    return true;
}
bool equalImage(const string& str1, const string& str2) {
    if (!isImage(str1) || !isImage(str2)) {
        cerr << "Error: input is not an image" << endl;
        return false;
    }
    Mat image1 = imread(str1);
    Mat image2 = imread(str2);
    if (image1.empty() || image2.empty()) {
        std::cerr << "Error loading images." << std::endl;
        return false;
    }
    Mat diff;
    absdiff(image1, image2, diff);

    // 计算差异图像的范数
    double normValue = norm(diff);
    // 设置一个阈值，如果范数小于阈值，认为图像相等
    double threshold = 0.0;
    return (normValue == threshold);
}

double cosineImageSimilarity(CKKS& cryptor, vector<Ciphertext>& ciphers, const string& image_dir, vector<Ciphertext>& result) {
    vector<string> image_paths;
    getFilePath(image_dir, image_paths);
    auto image_path = image_paths.begin();
    auto cipher = ciphers.begin();
    double ret = 1.0;
    while (image_path != image_paths.end() && cipher != ciphers.end()) {
        Ciphertext temp;
        cryptor.loadCiphertext(*image_path, temp);
        result.push_back(temp);
        double cos_s = cryptor.cosineSimilarity(*cipher, temp);
        if (cos_s < 0.999) {
            result = vector<Ciphertext>();
            return 0;
        }
        ret *= cos_s;
        image_path++;
        cipher++;
    }
    return ret;
}
long long getClockTime() {
    // 获取当前高分辨率时间点
    auto currentTimePoint = std::chrono::high_resolution_clock::now();

    // 将当前时间点转换为时间戳（以纳秒为单位）
    auto timestamp = std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTimePoint).time_since_epoch().count();

    // 输出时间戳
    //std::cout << "High-resolution Timestamp: " << timestamp << " ns" << std::endl;
    return timestamp;
}
void search(CKKS& cryptor, vector<Ciphertext>& ciphers, const string& image_dir, string& str, vector<Ciphertext>& result, double& count_time) {
    if (ciphers.empty()) {
        cerr << "Error: input is empty" << endl;
        str = "";
        result = vector<Ciphertext>();
        return;
    }
    vector<string> cipher_dir;
    getSubDir(image_dir, cipher_dir);
    for (auto& dir : cipher_dir) {
        long long start = getClockTime();
        double cos_s = cosineImageSimilarity(cryptor, ciphers, dir, result);
        long long end = getClockTime();
        /*ios old_fmt(nullptr);
        old_fmt.copyfmt(cout);
        cout << fixed << setprecision(10);
        cout << cos_s << endl;
        cout.copyfmt(old_fmt);*/
        if (cos_s > 0.9999) {
            str = dir;
            count_time = static_cast<double>(end - start) / 1000000;
            return;
        }
    }
    str = "";
    result = vector<Ciphertext>();
}
void evaluate(CKKS& cryptor, Ciphertext& cipher, double& add_time, double& mul_time, double& dot_time) {
    Ciphertext result;
    vector<double> add_times;
    vector<double> mul_times;
    vector<double> dot_times;
    for (int i = 0; i < 1000; i++) {
        long long time_0 = getClockTime();
        for (int j = 0; j < 1000; j++) {
            cryptor.add(cipher, cipher, result);
        }
        long long time_1 = getClockTime();
        cryptor.mul_vector(cipher, cipher, result);
        long long time_2 = getClockTime();
        cryptor.dot(cipher, cipher, result);
        long long time_3 = getClockTime();
        double add = static_cast<double>(time_1 - time_0) / 1000000000;
        double mul = static_cast<double>(time_2 - time_1) / 1000000;
        double dot = static_cast<double>(time_3 - time_2) / 1000000;
        add_times.push_back(add);
        mul_times.push_back(mul);
        dot_times.push_back(dot);
    }
    add_time = add_self(add_times) / add_times.size();
    mul_time = add_self(mul_times) / mul_times.size();
    dot_time = add_self(dot_times) / dot_times.size();
}