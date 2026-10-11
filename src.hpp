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
    std::map<int, Pokemon> pokemonMap;  // Sorted by id
    std::string fileName;
    
    // Type effectiveness matrix: attacker -> defender -> multiplier
    std::map<std::string, std::map<std::string, float>> typeEffectiveness;
    
    void initializeTypeEffectiveness() {
        // Initialize all to 1.0 (neutral)
        std::vector<std::string> types = {"normal", "fire", "water", "electric", "grass", "ice", "fighting", "ground", "flying"};
        for (const auto& attacker : types) {
            for (const auto& defender : types) {
                typeEffectiveness[attacker][defender] = 1.0f;
            }
        }
        
        // Set up effectiveness relationships based on problem description
        // Fire strong against grass (2x), weak against water (0.5x)
        typeEffectiveness["fire"]["grass"] = 2.0f;
        typeEffectiveness["fire"]["water"] = 0.5f;
        
        // Water strong against fire (2x), weak against electric (0.5x)
        typeEffectiveness["water"]["fire"] = 2.0f;
        typeEffectiveness["water"]["electric"] = 0.5f;
        
        // Electric strong against water (2x), weak against grass (0.5x), no effect against ground (0x)
        typeEffectiveness["electric"]["water"] = 2.0f;
        typeEffectiveness["electric"]["grass"] = 0.5f;
        typeEffectiveness["electric"]["ground"] = 0.0f;
        
        // Grass strong against water (2x), weak against fire (0.5x)
        typeEffectiveness["grass"]["water"] = 2.0f;
        typeEffectiveness["grass"]["fire"] = 0.5f;
        
        // Ice strong against grass (2x), weak against fighting (0.5x)
        typeEffectiveness["ice"]["grass"] = 2.0f;
        typeEffectiveness["ice"]["fighting"] = 0.5f;
        
        // Fighting strong against ice (2x), weak against normal (0.5x)
        typeEffectiveness["fighting"]["ice"] = 2.0f;
        typeEffectiveness["fighting"]["normal"] = 0.5f;
        
        // Ground no effect against flying (0x)
        typeEffectiveness["ground"]["flying"] = 0.0f;
    }
    
    bool isValidName(const char* name) {
        if (!name || strlen(name) == 0 || strlen(name) > 10) return false;
        for (size_t i = 0; i < strlen(name); i++) {
            if (!isalpha(name[i])) return false;
        }
        return true;
    }
    
    bool isValidType(const std::string& type) const {
        std::set<std::string> validTypes = {"normal", "fire", "water", "electric", "grass", "ice", "fighting", "ground", "flying"};
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
        initializeTypeEffectiveness();
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        try {
            // Validate name
            if (!isValidName(name)) {
                throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
            }
            
            // Check if name already exists
            for (const auto& pair : pokemonMap) {
                if (strcmp(pair.second.name, name) == 0) {
                    return false;  // Name already exists
                }
            }
            
            // Check if id already exists
            if (pokemonMap.find(id) != pokemonMap.end()) {
                return false;  // ID already exists
            }
            
            // Parse and validate types
            std::vector<std::string> typeList = parseTypes(types);
            
            // Add pokemon
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
                
                // Check if pokemon has all the search types
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
            
            // Sort by id (already sorted in map)
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
        
        std::string attackType(type);
        if (typeEffectiveness.find(attackType) == typeEffectiveness.end()) {
            return -1.0f;
        }
        
        const Pokemon& defender = it->second;
        float totalDamage = 1.0f;
        
        for (const auto& defType : defender.types) {
            if (typeEffectiveness.at(attackType).find(defType) != typeEffectiveness.at(attackType).end()) {
                totalDamage *= typeEffectiveness.at(attackType).at(defType);
            }
        }
        
        return totalDamage;
    }

    int catchTry() const {
        if (pokemonMap.empty()) {
            return 0;
        }
        
        // Start with the pokemon with smallest id
        std::set<int> ownedPokemon;
        ownedPokemon.insert(pokemonMap.begin()->first);
        
        bool newPokemonCaught = true;
        while (newPokemonCaught) {
            newPokemonCaught = false;
            
            for (const auto& pair : pokemonMap) {
                int targetId = pair.first;
                const Pokemon& target = pair.second;
                
                // Skip if already owned
                if (ownedPokemon.find(targetId) != ownedPokemon.end()) {
                    continue;
                }
                
                // Check if any owned pokemon can catch this target
                for (int ownedId : ownedPokemon) {
                    const Pokemon& attacker = pokemonMap.at(ownedId);
                    
                    // Try each type of the attacker
                    for (const auto& attackType : attacker.types) {
                        float damage = 1.0f;
                        
                        // Calculate damage against target
                        for (const auto& defType : target.types) {
                            if (typeEffectiveness.at(attackType).find(defType) != typeEffectiveness.at(attackType).end()) {
                                damage *= typeEffectiveness.at(attackType).at(defType);
                            }
                        }
                        
                        // If can deal at least 2x damage, can catch
                        if (damage >= 2.0f) {
                            ownedPokemon.insert(targetId);
                            newPokemonCaught = true;
                            break;
                        }
                    }
                    
                    if (newPokemonCaught) break;
                }
            }
        }
        
        return ownedPokemon.size();
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