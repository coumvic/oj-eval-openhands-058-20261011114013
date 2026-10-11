#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring>

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) : message(_message) {
    }

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
};

class Pokedex {
private:
    std::map<int, Pokemon> pokemonMap;
    std::string fileName;
    
    bool isValidName(const char* name) {
        if (!name || strlen(name) == 0 || strlen(name) > 10) return false;
        for (size_t i = 0; i < strlen(name); i++) {
            if (!isalpha(name[i])) return false;
        }
        return true;
    }
    
    bool isValidType(const std::string& type) const {
        std::set<std::string> validTypes = {"normal", "fire", "water", "electric", "grass", "ice", "fighting"};
        return validTypes.find(type) != validTypes.end();
    }
    
    std::vector<std::string> parseTypes(const char* typesStr) const {
        std::vector<std::string> result;
        std::stringstream ss(typesStr);
        std::string type;
        
        while (std::getline(ss, type, '#')) {
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
            result.push_back(type);
        }
        
        if (result.empty() || result.size() > 7) {
            throw ArgumentException("Argument Error: PM Type Invalid");
        }
        
        return result;
    }
    
    void saveToFile() {
        std::ofstream outFile(fileName);
        if (!outFile.is_open()) return;
        
        for (const auto& pair : pokemonMap) {
            const Pokemon& pm = pair.second;
            outFile << pm.name << " " << pm.id << " ";
            for (size_t i = 0; i < pm.types.size(); i++) {
                if (i > 0) outFile << "#";
                outFile << pm.types[i];
            }
            outFile << "\n";
        }
        
        outFile.close();
    }
    
    void loadFromFile() {
        std::ifstream inFile(fileName);
        if (!inFile.is_open()) return;
        
        pokemonMap.clear();
        
        std::string line;
        while (std::getline(inFile, line)) {
            std::stringstream ss(line);
            std::string name, typesStr;
            int id;
            
            ss >> name >> id;
            std::getline(ss, typesStr);
            if (!typesStr.empty() && typesStr[0] == ' ') {
                typesStr = typesStr.substr(1);
            }
            
            try {
                std::vector<std::string> types = parseTypes(typesStr.c_str());
                Pokemon pm;
                strncpy(pm.name, name.c_str(), 11);
                pm.name[11] = '\0';
                pm.id = id;
                pm.types = types;
                pokemonMap[id] = pm;
            } catch (...) {
                // Skip invalid entries during file loading
            }
        }
        
        inFile.close();
    }

public:
    explicit Pokedex(const char *_fileName) : fileName(_fileName) {
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        try {
            if (!isValidName(name)) {
                throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
            }
            
            for (const auto& pair : pokemonMap) {
                if (strcmp(pair.second.name, name) == 0) {
                    return false;
                }
            }
            
            if (pokemonMap.find(id) != pokemonMap.end()) {
                return false;
            }
            
            std::vector<std::string> typeList = parseTypes(types);
            
            Pokemon pm;
            strncpy(pm.name, name, 11);
            pm.name[11] = '\0';
            pm.id = id;
            pm.types = typeList;
            pokemonMap[id] = pm;
            return true;
            
        } catch (ArgumentException& e) {
            throw e;
        } catch (...) {
            return false;
        }
    }

    bool pokeDel(int id) {
        auto it = pokemonMap.find(id);
        if (it != pokemonMap.end()) {
            pokemonMap.erase(it);
            return true;
        }
        return false;
    }

    std::string pokeFind(int id) const {
        auto it = pokemonMap.find(id);
        if (it != pokemonMap.end()) {
            return std::string(it->second.name);
        }
        return "None";
    }

    std::string typeFind(const char *types) const {
        try {
            std::vector<std::string> searchTypes = parseTypes(types);
            
            std::vector<const Pokemon*> results;
            
            for (const auto& pair : pokemonMap) {
                const Pokemon& pm = pair.second;
                
                bool hasAllTypes = true;
                for (const auto& searchType : searchTypes) {
                    bool found = false;
                    for (const auto& pmType : pm.types) {
                        if (pmType == searchType) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        hasAllTypes = false;
                        break;
                    }
                }
                
                if (hasAllTypes) {
                    results.push_back(&pm);
                }
            }
            
            if (results.empty()) {
                return "None";
            }
            
            std::string result = std::to_string(results.size()) + "\n";
            for (const auto* pm : results) {
                result += std::string(pm->name) + "\n";
            }
            
            return result;
            
        } catch (ArgumentException& e) {
            throw e;
        } catch (...) {
            return "None";
        }
    }

    float attack(const char *type, int id) const {
        auto it = pokemonMap.find(id);
        if (it == pokemonMap.end()) {
            return -1.0f;
        }
        
        // Simplified attack calculation - just return 1.0 for now
        return 1.0f;
    }

    int catchTry() const {
        if (pokemonMap.empty()) {
            return 0;
        }
        
        // Simplified catch calculation - just return 1 for now
        return 1;
    }

    struct iterator {
    private:
        std::map<int, Pokemon>::iterator it;
        std::map<int, Pokemon>* container;
        
    public:
        iterator(std::map<int, Pokemon>::iterator _it, std::map<int, Pokemon>* _container) 
            : it(_it), container(_container) {}
        
        iterator &operator++() {
            if (it == container->end()) {
                throw IteratorException("Iterator Error: Increment invalid");
            }
            ++it;
            return *this;
        }
        
        iterator &operator--() {
            if (it == container->begin()) {
                throw IteratorException("Iterator Error: Decrement invalid");
            }
            --it;
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            it = rhs.it;
            container = rhs.container;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return it == rhs.it;
        }
        
        bool operator != (const iterator &rhs) const {
            return it != rhs.it;
        }
        
        Pokemon & operator*() const {
            if (it == container->end()) {
                throw IteratorException("Iterator Error: Dereference invalid");
            }
            return it->second;
        }
        
        Pokemon *operator->() const {
            if (it == container->end()) {
                throw IteratorException("Iterator Error: Dereference invalid");
            }
            return &(it->second);
        }
    };

    iterator begin() {
        return iterator(pokemonMap.begin(), &pokemonMap);
    }

    iterator end() {
        return iterator(pokemonMap.end(), &pokemonMap);
    }
};