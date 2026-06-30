#include "FirewallEvaluator.h"
#include <cmath> 

FirewallEvaluator::FirewallEvaluator() {}

float FirewallEvaluator::calcularCoseno(const float* v1, const float* v2, int dimensiones) {
    float productoPunto = 0.0f;
    float magnitudV1 = 0.0f;
    float magnitudV2 = 0.0f;

    for (int i = 0; i < dimensiones; ++i) {
        float val1 = *(v1 + i);
        float val2 = *(v2 + i);

        productoPunto += val1 * val2;
        magnitudV1 += val1 * val1;
        magnitudV2 += val2 * val2;
    }

    if (magnitudV1 == 0.0f || magnitudV2 == 0.0f) return 0.0f;

    return productoPunto / (std::sqrt(magnitudV1) * std::sqrt(magnitudV2));
}

bool FirewallEvaluator::evaluarSeguridad(const float* vectorActual,
    const float** vectoresBD,
    int dimensiones,
    int cantidadVectoresBD) {

    for (int i = 0; i < cantidadVectoresBD; ++i) {
        float similitud = calcularCoseno(vectorActual, vectoresBD[i], dimensiones);

        if (similitud >= 0.85f) {
               return true; 
        }
    }
    return false; 
}
