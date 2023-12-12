#include "examples.h"

//string image_path = ".\\resources\\miku.png";
int main() {
    string image_path1 = ".\\resources\\images\\test_0.png";
    string image_path2 = ".\\resources\\images\\test_1.png";
    string enc_dir = ".\\resources\\ciphers";
    string image_dir = ".\\resources\\images";
    string key_path = ".\\resources\\key";

    CKKS cryptor;
    size_t slot_count = cryptor.getSlot();
    cryptor.loadPrivate(key_path + "\\private");

    vector<double> input;
    input.reserve(slot_count);  // ָ��vector�Ĺ̶���С����push_backʱresize����Ч
    double curr_point = 0;
    double step_size = 1.0 / (static_cast<double>(slot_count) - 1); // ����4096 ���� 0 �� 1 �ĸ�����
    for (size_t i = 0; i < slot_count; i++)
    {
        input.push_back(curr_point);
        curr_point += step_size;
    }
    Ciphertext encrpted;
    double add_time, mul_time, dot_time;
    cryptor.encrypt(input, encrpted);
    evaluate(cryptor, encrpted, add_time, mul_time, dot_time);
    ios old_fmt(nullptr);
    old_fmt.copyfmt(cout);
    cout << fixed << setprecision(10);
    cout << "   / " << endl;
    cout << "   | " << "average add_time: " << add_time<< "ms" << endl;
    cout << "   | " << "average mul_time: " << mul_time << "ms" << endl;
    cout << "   | " << "average dot_time: " << dot_time << "ms" << endl;
    cout << "   \\ " << endl;
    cout.copyfmt(old_fmt);

    return 0;
    // ����Ϊ����ʱ�����
    vector<string> image_paths;
    getImagePath(image_dir, image_paths);

    vector<double>enc_times;
    //ios old_fmt(nullptr);
    old_fmt.copyfmt(cout);
    cout << fixed << setprecision(10);
    // ���� image_paths �е�����ͼ�񣬱����� enc_dir ��
    for (string& it : image_paths) {
        vector<Ciphertext> image_ciphers;
        long long start_time = getClockTime();
        cryptor.enc_image(it, image_ciphers);
        long long end_time = getClockTime();
        double enc_time = static_cast<double>(end_time - start_time) / 1000000;

        enc_times.push_back(enc_time);
        cout << "encrypt time: " << enc_time << "ms" << endl;

        fs::path image_path(it);
        string filename = image_path.stem().string();
        string save_dir = enc_dir + "\\" + filename;
        create_directory(save_dir);

        int i = 0;
        for (Ciphertext& cipher : image_ciphers) {
            string save_path = save_dir + "\\" + to_string(i) + ".dat";
            cryptor.saveCiphertext(save_path, cipher);
            i++;
        }
    }
    double ave_enc_time = add_self(enc_times)/enc_times.size();
    cout << "   /" << endl;
    cout << "   | average encrypt time: " << ave_enc_time << "ms" << endl;
    cout << "   \\" << endl;
    // ������ͼ����м��ܣ�ʹ�����ļ���enc_dir����֮��ƥ���ͼ��

    vector<double> search_times;
    vector<double> count_times;
    for (string& it : image_paths) {
        //vector<Ciphertext> image_ciphers;
        vector<Ciphertext> ciphers, result;
        string found_path;
        cryptor.enc_image(it, ciphers);

        // ����ƥ��ͼ��
        double count_time;
        long long start_time = getClockTime();
        search(cryptor, ciphers, enc_dir, found_path, result, count_time);
        long long end_time = getClockTime();
        double search_time = static_cast<double>(end_time - start_time) / 1000000;
        search_times.push_back(search_time);
        count_times.push_back(count_time);
        if (result.empty())
        {
            cout << "query image: " << it << " found no ciphers" << endl;
            continue;
        }
        cout << "   /" << endl;
        cout << "   | " << "query image: " << it << endl;
        cout << "   | " << "found ciphers in: " << found_path << endl;
        cout << "   | " << "search time: " << search_time << endl;
        cout << "   | " << "Similarity calculate time: " << count_time << endl;
        // ��֤���ҵ���ͼ���Ƿ���ȷ
        vector<vector<double>> imageVector;
        cryptor.dec_image(result, imageVector);
        if (equalImage(it, imageVector))
            cout << "   | " << "cipher found is right" << endl;
        else
            cout << "   | " << "cipher found is wrong" << endl;
        cout << "   \\" << endl;
    }
    double ave_search_time = add_self(search_times) / search_times.size();
    double ave_count_time = add_self(count_times) / count_times.size();
    cout << "   /" << endl;
    cout << "   | average search time: " << ave_search_time << "ms" << endl;
    cout << "   | average similarity calculate time: " << ave_count_time << "ms" << endl;
    cout << "   \\" << endl;
    cout.copyfmt(old_fmt);
    return 0;
}