#pragma once

#include <functional>

// j'ai pas réussi à faire avec un vrai switch + hash car switch prend que des constantes à la compilation
// ça c'est de la merde et y a même pas besoin du hash

#define	SWITCH(x) std::size_t __switch_hash__ = std::hash<decltype(x)>{}(x); if (false) {}

#define	CASE(x) else if (std::hash<decltype(x)>{}(x) == __switch_hash__)

#define DEFAULT else