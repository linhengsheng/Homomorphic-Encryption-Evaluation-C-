// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "seal/seal.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>

using namespace std;
using namespace cv;
using namespace seal;
namespace fs = std::filesystem;
using namespace fs;

void ImageShow(cv::String str);

void add(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>& result);
double dot(const vector<double>& vector_1, const vector<double>& vector_2);
void mul(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>& result);
double add_self(const vector<double>& vector);
bool isImage(string path);
void getImagePath(const string& dir, vector<string>& imagePaths);
void getImageVector(const string& str, vector<vector<double>>& result);
void getFilePath(const string& dir, vector<string>& Paths);
void getSubDir(const string& dir, vector<string>& Paths);
bool equalImage(const string& str1, const string& str2);
bool equalImage(const vector<vector<double>>& image1, const vector<vector<double>>& image2);
bool equalImage(const string& str, const vector<vector<double>>& input);
long long getClockTime();
/*
Helper function: Prints the name of the example in a fancy banner.
*/
inline void print_example_banner(std::string title)
{
    if (!title.empty())
    {
        std::size_t title_length = title.length();
        std::size_t banner_length = title_length + 2 * 10;
        std::string banner_top = "+" + std::string(banner_length - 2, '-') + "+";
        std::string banner_middle = "|" + std::string(9, ' ') + title + std::string(9, ' ') + "|";

        std::cout << std::endl << banner_top << std::endl << banner_middle << std::endl << banner_top << std::endl;
    }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
inline void print_parameters(const seal::SEALContext &context)
{
    auto &context_data = *context.key_context_data();

    /*
    Which scheme are we using?
    */
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::bfv:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::ckks:
        scheme_name = "CKKS";
        break;
    case seal::scheme_type::bgv:
        scheme_name = "BGV";
        break;
    default:
        throw std::invalid_argument("unsupported scheme");
    }
    std::cout << "/" << std::endl;
    std::cout << "| Encryption parameters :" << std::endl;
    std::cout << "|   scheme: " << scheme_name << std::endl;
    std::cout << "|   poly_modulus_degree: " << context_data.parms().poly_modulus_degree() << std::endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    std::cout << "|   coeff_modulus size: ";
    std::cout << context_data.total_coeff_modulus_bit_count() << " (";
    auto coeff_modulus = context_data.parms().coeff_modulus();
    std::size_t coeff_modulus_size = coeff_modulus.size();
    for (std::size_t i = 0; i < coeff_modulus_size - 1; i++)
    {
        std::cout << coeff_modulus[i].bit_count() << " + ";
    }
    std::cout << coeff_modulus.back().bit_count();
    std::cout << ") bits" << std::endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == seal::scheme_type::bfv)
    {
        std::cout << "|   plain_modulus: " << context_data.parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\" << std::endl;
}

/*
Helper function: Prints the `parms_id' to std::ostream.
*/
inline std::ostream &operator<<(std::ostream &stream, seal::parms_id_type parms_id)
{
    /*
    Save the formatting information for std::cout.
    */
    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    stream << std::hex << std::setfill('0') << std::setw(16) << parms_id[0] << " " << std::setw(16) << parms_id[1]
           << " " << std::setw(16) << parms_id[2] << " " << std::setw(16) << parms_id[3] << " ";

    /*
    Restore the old std::cout formatting.
    */
    std::cout.copyfmt(old_fmt);

    return stream;
}

/*
Helper function: Prints a vector of floating-point values.
*/
template <typename T>
inline void print_vector(std::vector<T> vec, std::size_t print_size = 4, int prec = 3)
{
    /*
    Save the formatting information for std::cout.
    */
    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    std::size_t slot_count = vec.size();

    std::cout << std::fixed << std::setprecision(prec);
    std::cout << std::endl;
    if (slot_count <= 2 * print_size)
    {
        std::cout << "    [";
        for (std::size_t i = 0; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    else
    {
        vec.resize(std::max(vec.size(), 2 * print_size));
        std::cout << "    [";
        for (std::size_t i = 0; i < print_size; i++)
        {
            std::cout << " " << vec[i] << ",";
        }
        if (vec.size() > 2 * print_size)
        {
            std::cout << " ...,";
        }
        for (std::size_t i = slot_count - print_size; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    std::cout << std::endl;

    /*
    Restore the old std::cout formatting.
    */
    std::cout.copyfmt(old_fmt);
}

/*
Helper function: Prints a matrix of values.
*/
template <typename T>
inline void print_matrix(std::vector<T> matrix, std::size_t row_size)
{
    /*
    We're not going to print every column of the matrix (there are 2048). Instead
    print this many slots from beginning and end of the matrix.
    */
    std::size_t print_size = 5;

    std::cout << std::endl;
    std::cout << "    [";
    for (std::size_t i = 0; i < print_size; i++)
    {
        std::cout << std::setw(3) << std::right << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = row_size - print_size; i < row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
    }
    std::cout << "    [";
    for (std::size_t i = row_size; i < row_size + print_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
    }
    std::cout << std::endl;
}

/*
Helper function: Print line number.
*/
inline void print_line(int line_number)
{
    std::cout << "Line " << std::setw(3) << line_number << " --> ";
}

/*
Helper function: Convert a value into a hexadecimal string, e.g., uint64_t(17) --> "11".
*/
inline std::string uint64_to_hex_string(std::uint64_t value)
{
    return seal::util::uint_to_hex_string(&value, std::size_t(1));
}


class CKKS {
public:
	CKKS(const EncryptionParameters& params = defaultEncryptionParameters(), const double& _scale = pow(2.0, 40))
		:parms(params), context(params), scale(_scale), keyGen(context), encoder(context), secret_key(keyGen.secret_key()), public_key(), relin_keys(), gal_keys() {
		keyGen.create_public_key(public_key);
		keyGen.create_relin_keys(relin_keys);
		keyGen.create_galois_keys(gal_keys);

		encryptor = make_unique<Encryptor>(context, public_key);
		evaluator = make_unique<Evaluator>(context);
		decryptor = make_unique<Decryptor>(context, secret_key);

		slot_count = encoder.slot_count();
	}
	double getScale() {
		return scale;
	}
	size_t getSlot() {
		return slot_count;
	}
	void getPublicKey(PublicKey& publicKey) {
		publicKey = public_key;
	}
	void savePublic(string str) {
		str = str + "\\public";
		if (!create_directories(str)) {
			// 创建失败
			cerr << "Failed to create directory: " << str << std::endl;
		}
		//else {
		//	// 文件夹创建成功
		//	cout << "Directory created successfully: " << str << std::endl;
		//}
		ofstream public_key_file(str + "\\public_key.txt", ios::binary);
		ofstream gal_keys_file(str + "\\gal_keys.txt", ios::binary);
		ofstream relin_keys_file(str + "\\relin_keys.txt", ios::binary);

		public_key.save(public_key_file);
		gal_keys.save(gal_keys_file);
		relin_keys.save(relin_keys_file);
		cout << "publicKeys are saved in " << str << endl;
	}
	void savePrivate(string str) {
		str = str + "\\private";
		if (!create_directories(str)) {
			// 创建失败
			cerr << "Failed to create directory: " << str << std::endl;
		}
		//else {
		//	// 文件夹创建成功
		//	cout << "Directory created successfully: " << str << std::endl;
		//}
		ofstream gal_keys_file(str + "\\gal_keys.txt", ios::binary);
		ofstream public_key_file(str + "\\public_key.txt", ios::binary);
		ofstream secret_key_file(str + "\\secret_key.txt", ios::binary);
		ofstream relin_keys_file(str + "\\relin_keys.txt", ios::binary);

		gal_keys.save(gal_keys_file);
		public_key.save(public_key_file);
		secret_key.save(secret_key_file);
		relin_keys.save(relin_keys_file);
		cout << "secretKeys are saved in " << str << endl;
	}
	void loadPrivate(const string str) {
		ifstream gal_keys_file(str + "\\gal_keys.txt", ios::binary);
		ifstream public_key_file(str + "\\public_key.txt", ios::binary);
		ifstream secret_key_file(str + "\\secret_key.txt", ios::binary);
		ifstream relin_keys_file(str + "\\relin_keys.txt", ios::binary);

		gal_keys.load(context, gal_keys_file);
		public_key.load(context, public_key_file);
		secret_key.load(context, secret_key_file);
		relin_keys.load(context, relin_keys_file);

		encryptor = make_unique<Encryptor>(context, public_key);
		evaluator = make_unique<Evaluator>(context);
		decryptor = make_unique<Decryptor>(context, secret_key);
	}
	void saveCiphertext(const string str, const Ciphertext& cipher) {
		ofstream ciphertext_file(str, ios::binary);
		if (ciphertext_file.is_open()) {
			// 将密文序列化到文件
			cipher.save(ciphertext_file);
			ciphertext_file.close();
			//cout << "Ciphertext saved successfully." << endl;
		}
		else {
			cerr << "Unable to open the file for writing." << endl;
			return;
		}
	}
	void loadCiphertext(const string str, Ciphertext& cipher) {
		ifstream loaded_ciphertext_file(str, ios::binary);
		if (loaded_ciphertext_file.is_open()) {
			// 从文件加载密文
			cipher.load(context, loaded_ciphertext_file);
			loaded_ciphertext_file.close();
			//cout << "Ciphertext loaded successfully." << endl;
		}
		else {
			cerr << "Unable to open the file for reading." << endl;
			return;
		}
	}
	void encrypt(const vector<double>& input, Ciphertext& result) {
		Plaintext x_plain;
		encoder.encode(input, scale, x_plain);
		encryptor->encrypt(x_plain, result);
	}
	void decrypt(const Ciphertext& cipher, vector<double>& result) {
		Plaintext plain;
		decryptor->decrypt(cipher, plain);
		encoder.decode(plain, result);
	}
	void add(Ciphertext& cipher_1, Ciphertext& cipher_2, Ciphertext& result) {
		parms_id_type id_1 = cipher_1.parms_id();
		parms_id_type id_2 = cipher_2.parms_id();
		if (cipher_1.parms_id() != cipher_2.parms_id()) {
			if (id_1 > id_2) {
				evaluator->mod_switch_to_inplace(cipher_1, id_2);
			}
			else {
				evaluator->mod_switch_to_inplace(cipher_2, id_1);
			}
		}
		evaluator->add(cipher_1, cipher_2, result);
	}
	void square(const Ciphertext& cipher, Ciphertext& result) {
		evaluator->square(cipher, result);
		evaluator->relinearize_inplace(result, relin_keys);
		evaluator->rescale_to_next_inplace(result);
	}
	void mul_vector(const Ciphertext& cipher_1, const Ciphertext& cipher_2, Ciphertext& result) {
		evaluator->multiply(cipher_1, cipher_2, result);
		evaluator->relinearize_inplace(result, relin_keys);
		evaluator->rescale_to_next_inplace(result);
	}
	void dot(Ciphertext& cipher_1, Ciphertext& cipher_2, Ciphertext& result) {
		parms_id_type id_1 = cipher_1.parms_id();
		parms_id_type id_2 = cipher_2.parms_id();
		if (cipher_1.parms_id() != cipher_2.parms_id()) {
			if (id_1 > id_2) {
				evaluator->mod_switch_to_inplace(cipher_1, id_2);
			}
			else {
				evaluator->mod_switch_to_inplace(cipher_2, id_1);
			}

		}
		mul_vector(cipher_1, cipher_2, result);
		for (int i = 1; i < slot_count; i <<= 1) { // 左移一位，相当于乘以2
			Ciphertext rotated;
			evaluator->rotate_vector(result, i, gal_keys, rotated);	// 对result做旋转操作，i是旋转偏移量，rotated存储结果
			evaluator->add_inplace(result, rotated);
		}
	}
	void enc_image(const string str, vector<Ciphertext>& result) {
		vector<vector<double>> imageMatrix;
		getImageVector(str, imageMatrix);
		if (imageMatrix.empty()) {
			cerr << "Error: can't read image" << endl;
			result = vector<Ciphertext>();
			return;
		}
		for (vector<double>& it : imageMatrix) {
			Ciphertext temp;
			encrypt(it, temp);
			result.push_back(temp);
		}
	}
	void dec_image(const vector<Ciphertext>& ciphers, vector<vector<double>>& result) {
		for (auto& it : ciphers) {
			vector<double> temp;
			decrypt(it, temp);
			result.push_back(temp);
		}
	}
	double cosineSimilarity(Ciphertext& cipher1, Ciphertext& cipher2) {
		Ciphertext result1, result2, result3;
		dot(cipher1, cipher2, result1);
		dot(cipher1, cipher1, result2);
		dot(cipher2, cipher2, result3);
		vector<double> vec1, vec2, vec3;
		decrypt(result1, vec1);
		decrypt(result2, vec2);
		decrypt(result3, vec3);
		double ret = vec1[0] / sqrt((vec2[0] * vec3[0]));
		return ret;
	}
private:
	static EncryptionParameters defaultEncryptionParameters() {
		EncryptionParameters params(seal::scheme_type::ckks);
		size_t poly_modulus_degree = 8192;
		params.set_poly_modulus_degree(poly_modulus_degree);
		params.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 40, 40, 60 }));
		return params;
	}

	EncryptionParameters parms;
	double scale;
	SEALContext context;
	KeyGenerator keyGen;
	CKKSEncoder encoder;

	SecretKey secret_key;
	PublicKey public_key;
	RelinKeys relin_keys;
	GaloisKeys gal_keys;

	unique_ptr<Encryptor> encryptor;
	unique_ptr<Evaluator> evaluator;
	unique_ptr<Decryptor> decryptor;

	size_t slot_count;
};

void search(CKKS& cryptor, vector<Ciphertext>& ciphers, const string& image_dir, string& str, vector<Ciphertext>& result, double& count_time);
void evaluate(CKKS& cryptor, Ciphertext& cipher, double& add_time, double& mul_time, double& dot_time);