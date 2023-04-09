#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>

//is implementation must be better, use stat!
size_t getFileSize(std::ifstream &file)
{
  file.seekg(0, file.end);
  size_t length = file.tellg();
  file.seekg(0, file.beg);
  return length;
}

int main(int argc, char **argv)
{
  using namespace std;
  //max 5MB key.
  enum { max_key_size = 5242880, buffer_size = 16 };
  ifstream key(argv[3], ios::in | ios::binary);
  if(!key.is_open()) {
    cerr << "Error open file: " << argv[3] << endl;
  }
  size_t key_size = getFileSize(key);
  if(key_size == 0 || key_size > max_key_size) {
    cerr << "File " << argv[3] << "erro with size file" << "Must be " <<
      "0 > key < " << max_key_size/1024/1024 << "MB" << endl;
    key.close();
    exit(EXIT_FAILURE);
  }
  char *key_buff = new char[key_size];
  key.read(key_buff, key_size);
  if(key.gcount() != key_size) {
    cerr << "Some error reading key file: " << argv[3] << endl;
    delete[] key_buff;
    key.close();
    exit(EXIT_FAILURE);
  }
  //add check key only ascii.
  //read all input file for array, then for i in array check
  //  number in 'char' and do cipher xor.
  ofstream ofs(argv[2], ios::out | ios::binary);
  if(!ofs.is_open()) {
    cerr << "Open error file: " << argv[2] << endl;
    delete[] key_buff;
    key.close();
    exit(EXIT_FAILURE);
  }
  ifstream ifs(argv[1], ios::in | ios::binary);
  if(!ifs.is_open()) {
    cerr << "Open error file: " << argv[1] << endl;
    delete[] key_buff;
    key.close();
    exit(EXIT_FAILURE);
  }
  char *ifs_buff = new char[buffer_size];
  ifs.read(ifs_buff, buffer_size);
  if(ifs.gcount() == 0) {
    cerr << "File " << argv[1] << 
      "is null size, specify the correct file." << endl;
    delete[] key_buff;
    delete[] ifs_buff;
    ifs.close();
    key.close();
    exit(EXIT_FAILURE);
  }
  using namespace std::chrono;
  // too slow implementation.
  auto start = high_resolution_clock::now();
  //for all input file
  char tmp;
  for(int bytes = ifs.gcount(); bytes > 0; bytes = ifs.gcount()) {
    //for buffer
    for(int i = 0; i < bytes; i++) {
      tmp = ifs_buff[i];
      for(int j = 0; j < key_size; j++) {
        tmp = tmp ^ key_buff[j]; 
      }
      ifs_buff[i] = tmp;
    }
    ofs.write(ifs_buff, bytes);
    ifs.read(ifs_buff, buffer_size);
  }
  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<seconds>(stop - start);
  cout << duration.count() << endl;
  delete[] key_buff;
  delete[] ifs_buff;
  ifs.close();
  key.close();
  ofs.close();
  return EXIT_SUCCESS;
}
