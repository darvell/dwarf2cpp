#pragma once

#include "elf.h"
#include <map>
#include <string>
#include <vector>

struct SymbolInfo
{
	std::string name;
	Elf32_Addr address;
	Elf32_Word size;
	bool is_function;
	bool is_global;
	
	SymbolInfo() : address(0), size(0), is_function(false), is_global(false) {}
	
	SymbolInfo(const std::string& n, Elf32_Addr addr, Elf32_Word sz, bool func, bool global)
		: name(n), address(addr), size(sz), is_function(func), is_global(global) {}
};

class SymbolTable
{
private:
	std::map<Elf32_Addr, SymbolInfo> address_to_symbol;
	std::map<std::string, SymbolInfo> name_to_symbol;
	bool loaded;

public:
	SymbolTable() : loaded(false) {}
	
	bool loadFromElf(ElfFile* elf)
	{
		if (!elf) return false;
		
		int symbol_count = 0;
		Elf32_Sym* symbols = elf->getSymbolTable(&symbol_count);
		
		if (!symbols || symbol_count == 0) {
			return false;
		}
		
		printf("Loading %d symbols from ELF symbol table...\n", symbol_count);
		
		int functions_loaded = 0;
		int variables_loaded = 0;
		
		for (int i = 0; i < symbol_count; i++) {
			Elf32_Sym* sym = &symbols[i];
			
			// Read symbol properties
			Elf32_Addr address = elf->read<Elf32_Addr>((void*)&sym->st_value);
			Elf32_Word size = elf->read<Elf32_Word>((void*)&sym->st_size);
			unsigned char info = sym->st_info;
			
			// Skip symbols without addresses or names
			if (address == 0) continue;
			
			const char* symbol_name = elf->getSymbolName(sym);
			if (!symbol_name || strlen(symbol_name) == 0) continue;
			
			// Determine symbol type
			unsigned char type = ELF32_ST_TYPE(info);
			unsigned char bind = ELF32_ST_BIND(info);
			
			bool is_function = (type == STT_FUNC);
			bool is_variable = (type == STT_OBJECT);
			bool is_global = (bind == STB_GLOBAL) || (bind == STB_WEAK);
			
			// Only store functions and global variables
			if (is_function || (is_variable && is_global)) {
				SymbolInfo symbol_info(symbol_name, address, size, is_function, is_global);
				
				address_to_symbol[address] = symbol_info;
				name_to_symbol[symbol_name] = symbol_info;
				
				if (is_function) functions_loaded++;
				else variables_loaded++;
			}
		}
		
		printf("Loaded %d functions and %d variables from symbol table\n", 
			   functions_loaded, variables_loaded);
		
		loaded = true;
		return true;
	}
	
	// Find symbol by address
	const SymbolInfo* findByAddress(Elf32_Addr address) const
	{
		if (!loaded) return nullptr;
		
		auto it = address_to_symbol.find(address);
		if (it != address_to_symbol.end()) {
			return &it->second;
		}
		
		return nullptr;
	}
	
	// Find symbol by name
	const SymbolInfo* findByName(const std::string& name) const
	{
		if (!loaded) return nullptr;
		
		auto it = name_to_symbol.find(name);
		if (it != name_to_symbol.end()) {
			return &it->second;
		}
		
		return nullptr;
	}
	
	// Find function containing an address (for addresses within function ranges)
	const SymbolInfo* findFunctionContaining(Elf32_Addr address) const
	{
		if (!loaded) return nullptr;
		
		const SymbolInfo* best_match = nullptr;
		Elf32_Addr best_distance = 0xFFFFFFFF;
		
		for (const auto& pair : address_to_symbol) {
			const SymbolInfo& symbol = pair.second;
			
			if (symbol.is_function && symbol.address <= address) {
				Elf32_Addr distance = address - symbol.address;
				
				// Check if address falls within function range (if size is known)
				if (symbol.size > 0) {
					if (distance < symbol.size && distance < best_distance) {
						best_match = &symbol;
						best_distance = distance;
					}
				} else {
					// If size is unknown, find closest function before this address
					if (distance < best_distance) {
						best_match = &symbol;
						best_distance = distance;
					}
				}
			}
		}
		
		return best_match;
	}
	
	// Get all symbols for debugging
	std::vector<SymbolInfo> getAllSymbols() const
	{
		std::vector<SymbolInfo> result;
		for (const auto& pair : address_to_symbol) {
			result.push_back(pair.second);
		}
		return result;
	}
	
	bool isLoaded() const { return loaded; }
};