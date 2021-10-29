/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_64.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lnicosia <lnicosia@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/05 16:05:36 by lnicosia          #+#    #+#             */
/*   Updated: 2021/09/23 09:21:42 by lnicosia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm.h"
#include "libft.h"
#include "options.h"
#include <limits.h>

/*
**	Frees the content of a t_sym
*/

void			delsym(void *file, size_t size)
{
	(void)size;
	(void)file;
}

void	set_symbol_type(t_sym *sym, char *ptr, Elf64_Ehdr *header,
Elf64_Shdr *shstr, Elf64_Shdr *shstrhdr, int opt)
{
	uint16_t	shndx;
	int			shndx_ok;

	if (sym->sym->st_shndx >= header->e_shnum)
	{
		shndx = (uint16_t)~sym->sym->st_shndx;
		//shndx = sym->sym->st_shndx;
		shndx_ok = 0;
	}
	else
	{
		shndx = sym->sym->st_shndx;
		shndx_ok = 1;
	}
	Elf64_Shdr *sheader = (Elf64_Shdr*) (ptr + header->e_shoff
		+ (header->e_shentsize * shndx));
	if (opt & OPT_VERBOSE)
	{
		ft_printf("------------------------------------------------\n");
		ft_printf("\tName = %u (%s)\n", (uint32_t)sym->sym->st_name,
		ptr + shstr->sh_offset + sym->sym->st_name);
		ft_printf("\tInfo = %d\n", sym->sym->st_info);
		ft_printf("\tOther = %d\n", sym->sym->st_other);
		ft_printf("\tSection = %hu", shndx);
		if (shndx_ok)
			ft_printf(" (%s)\n", ptr + shstrhdr->sh_offset + sheader->sh_name);
		else
		{
			//ft_printf(" (INVALID SECTION or if inversed: %s)\n",
			//ptr + shstrhdr->sh_offset + sheader->sh_name);
			switch (sym->sym->st_shndx)
			{
				case SHN_UNDEF:
					ft_printf(" (UNDEF)\n");
					break ;
				case SHN_ABS:
					ft_printf(" (ABS)\n");
					break ;
				case SHN_COMMON:
					ft_printf(" (COMMON)\n");
					break ;
			}
		}
		ft_printf("\tValue = %016x\n", sym->sym->st_value);
		ft_printf("\tSize = %lu\n", (uint64_t)sym->sym->st_size);
	}
	sym->type = 'r';
	switch (ELF64_ST_TYPE(sym->sym->st_info))
	{
		case STT_OBJECT:
			if (shndx_ok && sheader->sh_flags == 3)
				sym->type = 'd';
			break ;
		case STT_FUNC:
			sym->type = 't';
			break ;
	}
	if (sym->sym->st_value == 0)
		sym->type = 'u';
	if (shndx_ok && ft_strstr(ptr + shstrhdr->sh_offset + sheader->sh_name, "data"))
		sym->type = 'd';
	if (shndx_ok && (ft_strequ(ptr + shstrhdr->sh_offset + sheader->sh_name, ".rodata")
		|| ft_strequ(ptr + shstrhdr->sh_offset + sheader->sh_name, ".eh_frame")))
		sym->type = 'r';
	if (shndx_ok && (ft_strequ(ptr + shstrhdr->sh_offset + sheader->sh_name, ".text")
		|| ft_strequ(ptr + shstrhdr->sh_offset + sheader->sh_name, ".init_array")
		|| ft_strequ(ptr + shstrhdr->sh_offset + sheader->sh_name, ".fini_array")))
		sym->type = 't';
	if (shndx_ok && ft_strstr(ptr + shstrhdr->sh_offset + sheader->sh_name, "bss"))
		sym->type = 'b';
	switch (ELF64_ST_BIND(sym->sym->st_info))
	{
		case STB_WEAK:
			sym->type = 'w';
			if (shndx_ok && sheader->sh_flags != 0)
				sym->type = 'W';
			break ;
		case STB_GLOBAL:
			if (sym->type != 'w')
				sym->type = ft_toupper(sym->type);
			break ;
	}
}

void	print_symbols(t_dlist *lst, char *ptr, Elf64_Ehdr *header,
Elf64_Shdr *shstr, Elf64_Shdr *shstrhdr, int opt)
{
	t_sym		*sym;
	int			shndx_ok;
	int			padding;

	(void)shstr;
	if (header->e_type == ET_REL)
		padding = 8;
	else
		padding = 16;
	while (lst && lst->prev)
		lst = lst->prev;
	while (lst)
	{
		sym = (t_sym*)lst->content;
		if (sym->sym->st_shndx >= header->e_shnum)
			shndx_ok = 0;
		else
			shndx_ok = 1;
		Elf64_Shdr	*sheader = (Elf64_Shdr*) (ptr + header->e_shoff
		+ (header->e_shentsize * sym->sym->st_shndx));
		if (sym->sym->st_shndx != 0)
			ft_printf("%0*x", padding, sym->sym->st_value);
		else
			ft_printf("%*s", padding, "");
		if (opt & OPT_VERBOSE)
			ft_printf(" %3d", sym->sym->st_info);
		ft_printf(" %c", sym->type);
		ft_printf(" %s", sym->name);
		if (opt & OPT_VERBOSE)
		{
			ft_printf(" T =");
			switch (ELF64_ST_TYPE(sym->sym->st_info))
			{
				case STT_NOTYPE:
					ft_printf(" NOTYPE");
					break ;
				case STT_OBJECT:
					ft_printf(" OBJECT");
					break ;
				case STT_FUNC:
					ft_printf(" FUNC");
					break ;
				case STT_SECTION:
					ft_printf(" SECTION");
					break ;
				case STT_FILE:
					ft_printf(" FILE");
					break ;
				case STT_LOPROC:
					ft_printf(" LOPROC");
					break ;
				case STT_HIPROC:
					ft_printf(" HIPROC");
					break ;
			}
			ft_printf(" (%d)", ELF64_ST_TYPE(sym->sym->st_info));
			ft_printf(", B =");
			switch (ELF64_ST_BIND(sym->sym->st_info))
			{
				case STB_LOCAL:
					ft_printf(" LOCAL");
					break ;
				case STB_GLOBAL:
					ft_printf(" GLOBAL");
					break ;
				case STB_WEAK:
					ft_printf(" WEAK");
					break ;
				case STB_LOPROC:
					ft_printf(" LOPROC");
					break ;
				case STB_HIPROC:
					ft_printf(" HIPROC");
					break ;
			}
			ft_printf(" (%d)", ELF64_ST_BIND(sym->sym->st_info));
			ft_printf(", O = %d", ELF64_ST_VISIBILITY(sym->sym->st_other));
			ft_printf(", S = %d", sym->sym->st_size);
			ft_printf(", H = %d", sym->sym->st_shndx);
			if (shndx_ok)
			{
				ft_printf(" (%s), flags =", ptr + shstrhdr->sh_offset + sheader->sh_name);
				if (sheader->sh_flags & SHF_WRITE)
					ft_printf(" WRITE");
				if (sheader->sh_flags & SHF_ALLOC)
					ft_printf(" ALLOC");
				if (sheader->sh_flags & SHF_EXECINSTR)
					ft_printf(" EXECINSTR");
				if (sheader->sh_flags & SHF_MASKPROC)
					ft_printf(" MASKPROC");
				ft_printf(" (%d)", sheader->sh_flags);
			}
			else
			{
				switch (sym->sym->st_shndx)
				{
					case SHN_UNDEF:
					ft_printf(" (UNDEF)");
						break ;
					case SHN_ABS:
						ft_printf(" (ABS)");
						break ;
					case SHN_COMMON:
						ft_printf(" (COMMON)");
						break ;
				}
			}
		}
		ft_printf("\n");
		lst = lst->next;
	}
}

void	handle_64(char *file, char *ptr, long int file_size, int opt)
{
	Elf64_Ehdr	*header;
	Elf64_Shdr	*sheader;
	Elf64_Shdr	*shstrhdr;
	Elf64_Shdr	*shstr;
	Elf64_Off	i;
	uint64_t	j;
	uint64_t	sym_count;
	Elf64_Sym	*elf_sym;
	t_dlist		*lst;
	t_dlist		*new;
	t_sym		sym;
	size_t		expected_size;

	header = (Elf64_Ehdr*) ptr;
	if (opt & OPT_VERBOSE)
	{
		switch (header->e_type)
		{
			case ET_NONE:
				ft_printf("Unknown type\n");
				break;
			case ET_REL:
				ft_printf("Relocatable file\n");
				break;
			case ET_EXEC:
				ft_printf("Executable file\n");
				break;
			case ET_DYN:
				ft_printf("Shared object\n");
				break;
			case ET_CORE:
				ft_printf("Core file\n");
				break;
		}
		ft_printf("%hu program entries\n", (uint16_t)header->e_phnum);
		ft_printf("%hu sections entries\n", (uint16_t)header->e_shnum);
		ft_printf("Section headers offset = %lu\n", (Elf64_Off)header->e_shoff);
		ft_printf("Strings section header index = %hu\n", (uint16_t)header->e_shstrndx);
		ft_printf("Sections header's size = %hu (total size = %d)\n", (uint16_t)header->e_shentsize,
		header->e_shentsize * header->e_shnum);
		ft_printf("Program header's size = %hu (total size = %d)\n",
		(uint16_t)header->e_phentsize, header->e_phentsize * header->e_phnum);
			ft_printf("Endian = ");
			if (ptr[5] == ELFDATA2LSB)
				ft_printf("little\n");
			else if (ptr[5] == ELFDATA2MSB)
				ft_printf("big\n");

	}
	// Check if the file is big enough to contain all the section headers
	if ((long int)header->e_shoff + header->e_shentsize * header->e_shnum > file_size
		|| (long int)header->e_phoff + header->e_phentsize * header->e_phnum > file_size)
	{
		custom_error("%s: file too short\n", file);
		custom_error("ft_nm: %s: File truncated\n", file);
		return ;
	}
	shstrhdr = (Elf64_Shdr*)(ptr + header->e_shoff + (header->e_shentsize * header->e_shstrndx));
	lst = NULL;
	new = NULL;
	sym.sym = NULL;
	sym.name = NULL;
	sym_count = 0;
	/*if (opt & OPT_VERBOSE)
	{
		ft_printf("Section types:\n");
		ft_printf("SHT_NULL = %d\n", SHT_NULL);
		ft_printf("SHT_PROGBITS = %d\n", SHT_PROGBITS);
		ft_printf("SHT_SYMTAB = %d\n", SHT_SYMTAB);
		ft_printf("SHT_STRTAB = %d\n", SHT_STRTAB);
		ft_printf("SHT_RELA = %d\n", SHT_RELA);
		ft_printf("SHT_HASH = %d\n", SHT_HASH);
		ft_printf("SHT_DYNAMIC = %d\n", SHT_DYNAMIC);
		ft_printf("SHT_NOTE = %d\n", SHT_NOTE);
		ft_printf("SHT_REL = %d\n", SHT_REL);
		ft_printf("SHT_SHLIB = %d\n", SHT_SHLIB);
		ft_printf("SHT_DYNSYM = %d\n", SHT_DYNSYM);
		ft_printf("SHT_LOPROC = %d\n", SHT_LOPROC);
		ft_printf("SHT_HIPROC = %d\n", SHT_HIPROC);
		ft_printf("SHT_LOUSER = %d\n", SHT_LOUSER);
		ft_printf("SHT_HIUSER = %d\n", SHT_HIUSER);
	}*/
	expected_size = (size_t)header->e_shentsize * header->e_shnum
					+ (size_t)header->e_phentsize * header->e_phnum
					+ header->e_ehsize;
	i = 0;
	while (i < header->e_shnum)
	{
		sheader = (Elf64_Shdr*) (ptr + header->e_shoff
		+ (header->e_shentsize * i));
		expected_size += sheader->sh_size;
		i++;
	}
	i = 0;
	while (i < header->e_phnum)
	{
		//Elf64_Phdr *pheader = (Elf64_Phdr*) (ptr + header->e_phoff
		//+ (header->e_phentsize * i));
		//file_size += pheader->p_filesz;
		//file_size += pheader->p_memsz;
		i++;
	}
	if (opt & OPT_VERBOSE)
		ft_printf("File expected size = %d\n", expected_size);
	i = 0;
	while (i < header->e_shnum)
	{
		sheader = (Elf64_Shdr*) (ptr + header->e_shoff
		+ (header->e_shentsize * i));
		// Check if the file is big enough to contain the section
		if ((long int)sheader->sh_offset + (long int)sheader->sh_size > file_size)
		{
			custom_error("%s: file too short\n", file);
			custom_error("ft_nm: %s: File truncated\n", file);
			return ;
		}
		if (opt & OPT_VERBOSE)
		{
			ft_printf("------------------------------\n");
			ft_printf("[Section %d]\n", i);
			ft_printf("Section name = %u (%s)\n", (uint32_t)sheader->sh_name,
			ptr + shstrhdr->sh_offset + sheader->sh_name);
			ft_printf("Section type = %u\n", (uint32_t)sheader->sh_type);
			ft_printf("Section flags = %u\n", (uint32_t)sheader->sh_flags);
			ft_printf("Section size = %lu\n", (uint64_t)sheader->sh_size);
			ft_printf("Section link = %u\n", (uint32_t)sheader->sh_link);
			ft_printf("Section info = %u\n", (uint32_t)sheader->sh_info);
			ft_printf("Section offset = %lu\n", (uint64_t)sheader->sh_offset);
			ft_printf("Entry size = %lu\n", (uint64_t)sheader->sh_entsize);
		}
		if (sheader->sh_type == SHT_SYMTAB)
		{
			sym_count += sheader->sh_size / sheader->sh_entsize;
			j = 0;
			if (opt & OPT_VERBOSE)
				ft_printf("Symbol section has %d symbols\n",
				sheader->sh_size / sheader->sh_entsize);
			shstr = (Elf64_Shdr*)(ptr + header->e_shoff + (header->e_shentsize * sheader->sh_link));
			while (j < sheader->sh_size / sheader->sh_entsize)
			{
				ft_bzero(&sym, sizeof(sym));
				elf_sym = (Elf64_Sym *)(ptr + sheader->sh_offset + (sheader->sh_entsize * j));
				if ((elf_sym->st_info == STT_FILE && header->e_type != ET_REL) || 
					elf_sym->st_info == STT_SECTION
					|| elf_sym->st_shndx == SHN_COMMON
					|| (elf_sym->st_info == 0 && elf_sym->st_value == 0))
				{
					j++;
					continue;
				}
				sym.sym = elf_sym;
				sym.name = ptr + shstr->sh_offset + elf_sym->st_name;
				set_symbol_type(&sym, ptr, header, shstr, shstrhdr, opt);
				if (!(new = ft_dlstnew(&sym, sizeof(sym))))
				{
					custom_error("ft_lstnew:");
					ft_dlstdelfront(&lst, delsym);
					return ;
				}
				if ((opt & OPT_C))
					ft_dlstinsert(&lst, new, compare_names);
				else
					ft_dlstinsert(&lst, new, compare_names);
				j++;
			}
		}
		else if (sheader->sh_type == SHT_STRTAB && opt & OPT_VERBOSE)
		{
			ft_printf("{cyan}String table{reset}\n");
		}
		i++;
	}
	if (opt & OPT_PRINT_FILE_NAME)
		ft_printf("\n%s:\n", file);
	print_symbols(lst, ptr, header, shstr, shstrhdr, opt);
	ft_dlstdelfront(&lst, delsym);
	if (sym_count == 0)
		custom_error("ft_nm: %s: no symbols\n", file);
}
