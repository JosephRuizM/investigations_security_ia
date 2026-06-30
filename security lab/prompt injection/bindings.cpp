#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <stdexcept>
#include "FirewallEvaluator.h"

namespace py = pybind11;

bool verificar_seguridad_numpy(py::array_t<float> prompt_actual, py::array_t<float> matriz_bd) {
    if (prompt_actual.size() == 0 || matriz_bd.size() == 0) {
        return false;
    }

    FirewallEvaluator evaluador;

    py::buffer_info info_actual = prompt_actual.request();
    float* ptr_actual = static_cast<float*>(info_actual.ptr);
    
    if (info_actual.ndim != 1) {
        throw std::runtime_error("Error dimensional: El vector del prompt actual debe ser de una sola dimensión (1D).");
    }
    int dimensiones = static_cast<int>(info_actual.shape[0]); 

    py::buffer_info info_bd = matriz_bd.request();
    float* ptr_bloque_bd = static_cast<float*>(info_bd.ptr);
    
    // Los ataques de la BD deben venir estructurados como una matriz bidimensional (2D)
    if (info_bd.ndim != 2) {
        throw std::runtime_error("Error dimensional: La matriz de la base de datos debe ser de dos dimensiones (2D).");
    }
    int cantidad_vectores = static_cast<int>(info_bd.shape[0]); 
    int dimensiones_bd = static_cast<int>(info_bd.shape[1]);

    if (dimensiones != dimensiones_bd) {
        throw std::runtime_error("Conflicto de configuración: La dimensión del prompt (" + std::to_string(dimensiones) + 
                                 ") no coincide con la de la BD (" + std::to_string(dimensiones_bd) + ").");
    }


    const float** vectores_bd = new const float* [cantidad_vectores];
    for (int i = 0; i < cantidad_vectores; ++i) {
        vectores_bd[i] = ptr_bloque_bd + (i * dimensiones);
    }

    bool resultado = evaluador.evaluarSeguridad(ptr_actual, vectores_bd, dimensiones, cantidad_vectores);

    delete[] vectores_bd;

    return resultado;
}

PYBIND11_MODULE(motor_firewall, m) {
    m.doc() = "Modulo de alto rendimiento en C++ para detectar Prompt Injection";
    m.def("evaluar_prompt", &verificar_seguridad_numpy, "Recibe arrays de NumPy y calcula la similitud de coseno en C++");
}
