#include <iostream>
#include <fstream>

extern "C" {
#include <getopt.h>
}

enum { max_key_size = 5242880, buffer_size = 16 };
const std::string help_message = "xorenc <enc [-i -o -k] | dec [-i -o -k] | keygen [-o -k]>\n\
  xorenc encrypted, decrypted  files based on the key. It also generate key.\n\
  it uses mandatory arguments in bundle with optional arguments.\n\
  example usage: xorenc keygen -s 2k -o mykey.bin\n\
  example usage: xorenc enc -i <input file> -o <output file> -k <keyfile> - where:\n\
  input file - what I want to encrypt, to path;\n\
  output file - what I want to decrypt, to path\n\
  keyfile - path to key file(can be any binary file in the range from 1 kilobytes to 5 kilobytes.\n\
  The size can be specified in bytes by default).\n\
  By default name output file 'encrypted' in ecnryption(enc) mode \n\
  By default name output file 'decrypted' in decryption(dec) mode \n\
  By default name output in keygen mode keyfile.bin, the file format is binary.\n\
  The names of the input and output files must not match!";
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
  std::ofstream out(str, std::ios::out | std::ios::binary);
  if(!out.is_open()) {
    std::cerr << "Error to create file '" << str << "'." << std::endl;
    exit(EXIT_FAILURE);
  }
  srand(time(0));
  for(unsigned int i = 0; i < size; i++) {
    // ascii charecters 
    out.put(0 + rand() % 126);
  }
  out.close();
}

void key_read(char *&key_buff, unsigned int &keysize, const std::string &file)
{
  using namespace std;
  ifstream key(file, ios::in | ios::binary);
  if(!key.is_open()) {
    cerr << "Error open file: " << file << endl;
  }
  keysize = getFileSize(key);
  if(keysize == 0 || keysize > max_key_size) {
    cerr << "File " << file << "too big! " << "Must be " <<
      "0 > key < " << max_key_size << "bytes" << endl;
    key.close();
    exit(EXIT_FAILURE);
  }
  key_buff = new char[keysize];
  key.read(key_buff, keysize);
  if(key.gcount() != keysize) {
    cerr << "Some error reading key file: " << file << endl;
    delete[] key_buff;
    exit(EXIT_FAILURE);
  }
  // space for improvements
  key.close();
}

//same decrypt, xor
void
encrypt(const std::string &out, const std::string &inp,
    char *&key_buff, unsigned int keysize)
{
  using namespace std;
  ofstream ofs(out, ios::out | ios::binary);
  if(!ofs.is_open()) {
    cerr << "Open error file: " << out << endl;
    delete[] key_buff;
    exit(EXIT_FAILURE);
  }
  ifstream ifs(inp, ios::in | ios::binary);
  if(!ifs.is_open()) {
    cerr << "Open error file: " << inp << endl;
    delete[] key_buff;
    exit(EXIT_FAILURE);
  }
  char *ifs_buff = new char[buffer_size];
  ifs.read(ifs_buff, buffer_size);
  if(ifs.gcount() == 0) {
    cerr << "File " << inp << 
      "is null size, specify the correct file." << endl;
    delete[] key_buff;
    delete[] ifs_buff;
    ifs.close();
    exit(EXIT_FAILURE);
  }
  char tmp;
  for(int bytes = ifs.gcount(); bytes > 0; bytes = ifs.gcount()) {
    for(int i = 0; i < bytes; i++) {
      tmp = ifs_buff[i];
      for(int j = 0; j < keysize; j++) {
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
  ofs.close();
}

int main(int argc, char **argv)
{
  using namespace std;
  if(argc < 2) {
    cout << mini_help << endl;
  }
  const char *short_opt = "hi:o:s:k:";
  const struct option long_opt[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"input", required_argument, 0, 'i'},
    {"size", required_argument, 0, 's'},
    {"key", required_argument, 0, 'k'},
    {0, 0, 0, 0}
  };
  static const char *mandatory_args[] = {"enc", "dec", "keygen"};
  string outfile(""), infile(""), keyfile("");
  unsigned int keysize = 0;
  char *key_buff;
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
    case 'i': {
      infile = optarg;
      break;
    }
    case 's': {
      // may be an exit.
      parse_key_val(optarg, keysize, type);
      break;
    }
    case 'k': {
      keyfile = optarg;
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
    // add check if key same encryption or decryption file: TODO
    if(!arg.compare(mandatory_args[0])) { // enc
      if(outfile.empty()) {
        outfile = "encrypted";
      }
      if(infile.empty()) {
        cerr << "Missing input file!" << endl << mini_help << endl;
        exit(EXIT_FAILURE);
      }
      if(infile == outfile) {
        cerr << "The nameos of file encyption and decryption are the same! "
          << "Rename them." << mini_help << endl;
        exit(EXIT_FAILURE);
      }
      if(keyfile.empty()) {
        cerr << "Missing key file! Use flag -k or --key." << endl <<
          mini_help << endl;
        exit(EXIT_FAILURE);
      }
      key_read(key_buff, keysize, keyfile);
      encrypt(outfile, infile, key_buff, keysize);
    } else if(!arg.compare(mandatory_args[1])) { // dec
      if(outfile.empty()) {
        outfile = "decrypted";
      }
      if(infile.empty()) {
        cerr << "Missing input file!" << endl << mini_help << endl;
        exit(EXIT_FAILURE);
      }
      if(infile == outfile) {
        cerr << "The nameos of file encyption and decryption are the same! " <<
          "Rename them." << mini_help << endl;
        exit(EXIT_FAILURE);
      }
      if(keyfile.empty()) {
        cerr << "Missing key file! Use flag -k or --key." << endl <<
          mini_help << endl;
        exit(EXIT_FAILURE);
      }
      key_read(key_buff, keysize, keyfile);
      encrypt(outfile, infile, key_buff, keysize);
    } else if(!arg.compare(mandatory_args[2])) { // keygen
      if(outfile.empty()) {
        outfile = "keyfile.bin";
      }
      if(!keysize) {
        cerr << "Define key size! use flag -s or --size" << endl;
      }
      if(type == 'k')
        gen_key(keysize*1024, outfile);
      else {
        gen_key(keysize, outfile);
      }
    } else {
      cerr << "'" << argv[i] << "'" << 
        " there is no such mandatory argument!" << endl << mini_help << endl;
    }
  }
  return EXIT_SUCCESS;
}
