#pragma once
class FirewallEvaluator
{
public:
	FirewallEvaluator();
	float calcularCoseno(const float* v1, const float* v2, int dimensiones);
	bool evaluarSeguridad(const float* vectorActual,
		const float** vectoresBD,int dimensiones,int cantidadVectoresBD);
};

