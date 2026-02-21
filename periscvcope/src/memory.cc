#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <elf.h>

#include <memory.hh>

using namespace mem;

void
memory::load_binary(const std::string& binfile)
{
    using ifile_iter = std::istream_iterator<uint8_t>;

    std::ifstream ifile(binfile, std::ios::binary);

    if (ifile.is_open() == false) {
	std::cerr << "Unable to open "<< binfile << std::endl;
	std::exit(EXIT_FAILURE);
    }

    // copy the binary into memory
    // Stop eating new lines in binary mode!!!
    ifile.unsetf(std::ios::skipws);

    ifile.seekg(0, std::ios::end);
    const auto ifsize = ifile.tellg();
    ifile.seekg(0, std::ios::beg);

    _binary.reserve(ifsize);
    _binary.insert(_binary.begin(),
	    ifile_iter(ifile),
	    ifile_iter());
    ifile.close();


    // read elf header
    // El primer paso a realizar es el de leer la cabecera del binario.
    // En memory.hh está definido Elf32_Ehdr _ehdr; // ELF header para saber el tipo de dató
    const Elf32_Ehdr* _ehdr = *reinterpret_cast<Elf32_Ehdr*>(_binary.data());

    // ensure riscv32
    //magic number
    assert(_ehdr->e_ident[EI_MAG0] == ELFMAG0);
    assert(_ehdr->e_ident[EI_MAG1] == ELFMAG1);
    assert(_ehdr->e_ident[EI_MAG2] == ELFMAG2);
    assert(_ehdr->e_ident[EI_MAG3] == ELFMAG3);  

    // ensure 32-bit
    assert(_ehdr->e_ident[EI_CLASS] == ELFCLASS32);

    //Little endian
    assert(_ehdr->e_ident[EI_DATA] == ELFDATA2LSB);

    //RISC-V
    // assert(_ehdr.e_machine == EM_RISCV); ?? no lo tengo claro, no se si es necesario, pero lo dejo comentado por ahora

    // ensure the binary has a correct program table


    // entry point
    _entry_point = _ehdr->e_entry;
    // load sections in memory

    // read ELF program header table,
    const Elf32_Phdr* _phdr = reinterpret_cast<const Elf32_Phdr*>(_binary.data() + _ehdr->e_phoff);
    const size_t phdr_size = _ehdr->e_phentsize;
    const size_t phdr_count = _ehdr->e_phnum;

    for (size_t i = 0; i < phdr_count; ++i) {
        // load each segment into memory
        if (_phdr[i].p_type == PT_LOAD) {
            const auto offset = _phdr[i].p_offset;
            const auto size = _phdr[i].p_filesz;
            const auto vaddr = _phdr[i].p_vaddr;

            // ensure we have enough space in memory
            if (vaddr + size > _memory.size()) {
            _memory.resize(vaddr + size);
            }

            // copy segment data into memory
            std::memcpy(&_memory[vaddr], &_binary[offset], size);
        }
    }
    // ... to be completed
}
