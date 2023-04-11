#include <iostream>
#include <fstream>

extern "C" {
#include <getopt.h>
}

enum { max_key_size = 5242880, buffer_size = 16 };
const std::string help_message = "Fully help.";
const std::string mini_help = "Usage: flag -h or --help for help";

//is implementation must be better, use stat!
size_t getFileSize(std::ifstream &file)
{
  file.seekg(0, file.end);
  size_t length = file.tellg();
  file.seekg(0, file.beg);
  return length;
}

void parse_key_val(const char *str, unsigned int &keysize, char &type)
{
  using namespace std;
  char *endptr;
  keysize = strtol(str, &endptr, 10);
  if(*endptr) {
    if(*endptr == 'k') {
      if(keysize == 0 || keysize > 5) {
        cerr << "Invalid specified range kilobytes, '" << keysize 
          << "' " << "should be 0 > kilobytes < 5." << endl;
        exit(EXIT_FAILURE);
      }
      type = 'k';
    } else {
      cerr << "Invalid key size'" << endptr << "'" << endl <<
        mini_help << endl;
      exit(EXIT_FAILURE);
    }
  }
  if(keysize == 0 || keysize > max_key_size) {
    cerr << "Invalid specified range bytes, '" << keysize << "'"
      << " should be 0 > bytes < " << max_key_size <<
      endl << mini_help << endl;
    exit(EXIT_FAILURE);
  }

}

void gen_key(unsigned int size, const std::string &str)
{
  using namespace std;
  enum {buff_size = 8};
  std::ofstream out(str, std::ios::out | std::ios::binary);
  if(!out.is_open()) {
    std::cerr << "Error to create file '" << str << "'." << std::endl;
    exit(EXIT_FAILURE);
  }
  srand(time(0));
  char random;
  int randv;
  char buff[buff_size];
  for(unsigned int i = 0, j; i < size; i += buff_size) {
    for(j = 0; j < i; j++) {
      cout << "i: " << i << " j: " << j << endl;
      randv = rand();
      random = 10 + randv % 126;
      buff[j] = random;
    }
    out.write(buff, j);
  }
  out.close();
}

int main(int argc, char **argv)
{
  using namespace std;
  //max 5MB key.
  const char *short_opt = "ho:k:";
  const struct option long_opt[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"key-size", required_argument, 0, 'k'},
    {0, 0, 0, 0}
  };
  static const char *mandatory_args[] = {"enc", "dec", "keygen"};
  string outfile(""), infile("");
  unsigned int keysize = 0;
  char type = 'B';
  int res, opt_idx;
  while((res = getopt_long(argc, argv, short_opt, long_opt, &opt_idx)) != -1) {
    switch(res) {
    case 'h': {
      cout << help_message << endl;
      exit(EXIT_SUCCESS);
    }
    case 'o': {
      outfile = optarg;
      break;
    }
    case 'k': {
      // may be an exit.
      parse_key_val(optarg, keysize, type);
      break;
    }
    case '?': default: {
      cout << "! " << optarg << endl;
      cerr << "Option argument is not exists!" << endl << mini_help << endl;
      exit(EXIT_FAILURE);
    }
    }
  }
  string arg;
  for(int i = optind; i < argc; i++) {
    arg = string(argv[i]);
    // maybe better use map for arg ? : TODO
    if(!arg.compare(mandatory_args[0])) {
      break;
    } else if(!arg.compare(mandatory_args[1])) {
      break;
    } else if(!arg.compare(mandatory_args[2])) {
      if(outfile.empty()) {
        outfile = "keyfile.bin";
      }
      if(!keysize) {
        cerr << "Define key size! use flag -k or --key-size" << endl;
      }
      if(type == 'k')
        gen_key(keysize*1024, outfile);
      else {
        gen_key(keysize, outfile);
      }
      return EXIT_SUCCESS;
    } else {
      cerr << "'" << argv[i] << "'" << 
        " there is no such mandatory argument!" << endl << mini_help << endl;
    }
  }
  exit(0);
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
  delete[] key_buff;
  delete[] ifs_buff;
  ifs.close();
  key.close();
  ofs.close();
  return EXIT_SUCCESS;
}
