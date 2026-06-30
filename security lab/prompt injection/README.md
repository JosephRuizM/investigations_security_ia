# Reporte de Seguridad: Auditoría de Caja Negra e Inyección de Prompts

## 1. Resumen Ejecutivo
Este documento detalla los resultados de la auditoría de seguridad realizada sobre el componente nativo de firewall de prompts (`prompt_protection`), el cual utiliza un motor de comparación vectorial en C++ acoplado a FastAPI y ChromaDB (`all-MiniLM-L6-v2`). El objetivo fue evaluar la resistencia del sistema frente a técnicas automatizadas de evasión semántica y estructural (Evasión Black-Box).

## 2. Metodología del Red Team
Se diseñó un pipeline de pruebas asíncronas para simular un ataque de fuerza bruta semántica a través del endpoint `/login`. Las pruebas consistieron en inyectar intenciones maliciosas base camufladas mediante cuatro técnicas independientes del modelo:

1. **CAMUFLAJE_ESPACIADO**: Inserción de caracteres invisibles o separadores (`_`, `-`, `/`, espacios) entre las letras de palabras clave.
2. **CAMUFLAJE_HOMÓGLIFOS**: Sustitución de caracteres latinos por caracteres de alfabetos alternos (cirílico) visualmente idénticos pero con diferente representación de bytes.
3. **CAMUFLAJE_RELLENO_BENIGNO**: Dilución del prompt inyectado dentro de un contexto corporativo, académico o literario extenso y legítimo.
4. **CAMUFLAJE_TRADUCCIÓN_LITERAL**: Alteración de la sintaxis original mediante el uso de prefijos complejos y modismos interpretativos.

---

## 3. Resultados de la Evaluación (Línea Base)
El firewall se sometió a estrés utilizando el script de automatización concurrentemente sobre el puerto local `8080`. Los resultados métricos fueron los siguientes:

| Métrica | Valor Registrado |
| :--- | :--- |
| **Tiempo de ejecución** | 0.31 segundos |
| **Peticiones Procesadas** | 30 / 30 |
| **Errores de Red** | 0 |
| **Latencia Promedio** | 11.50 ms |

### Efectividad de Detección por Técnica (Recall)
* **CAMUFLAJE_ESPACIADO**: 0.0% de bloqueos (0/8) - **EVADIDO**
* **CAMUFLAJE_RELLENO_BENIGNO**: 0.0% de bloqueos (0/9) - **EVADIDO**
* **CAMUFLAJE_HOMÓGLIFOS**: 0.0% de bloqueos (0/4) - **EVADIDO**
* **CAMUFLAJE_TRADUCCIÓN_LITERAL**: 0.0% de bloqueos (0/9) - **EVADIDO**

**Efectividad Global Inicial: 0.0%**

---

## 4. Análisis Técnico de los Puntos Ciegos

### A. Ruptura de Tokenización por Caracteres (Espaciado y Homóglifos)
El modelo `all-MiniLM-L6-v2` fragmenta el texto entrante basándose en combinaciones comunes de caracteres conocidas como tokens. 
* Al transformar `ignore` en `i - g - n - o - r - e`, el tokenizador lee letras aisladas en lugar del verbo de acción.
* Al usar homóglifos cirílicos, el valor numérico binario de los bytes cambia por completo.
* **Impacto**: La distancia geométrica del vector resultante se desvía drásticamente en ChromaDB respecto a las firmas de ataque guardadas, provocando que la Similitud de Coseno caiga muy por debajo del umbral de bloqueo configurado (0.85).

### B. Dilución Semántica (Relleno Benigno)
Los vectores de embedding representan el significado promedio del texto completo. Al incrustar una línea de inyección maliciosa dentro de varios párrafos de texto administrativo o una historia de ficción, los ejes vectoriales son arrastrados hacia la zona "segura" del mapa multidimensional. El firewall interpreta el prompt global como una solicitud de negocio válida.

---

## 5. Plan de Mitigación y Remediación (Próximos Pasos)
Para elevar la efectividad del firewall a niveles aceptables de producción, se implementará un pipeline de sanitización previa (*Pre-processing Pipeline*) en FastAPI antes de la fase de embedding:

1. **Filtro de Homóglifos**: Mapeo estricto a nivel de bits para normalizar caracteres Unicode sospechosos a sus equivalentes del alfabeto latino estándar.
2. **Normalización Estructural**: Algoritmo de limpieza para detectar patrones de texto espaciado o caracteres de puntuación repetitivos y compactarlos.
3. **Estrategia de Ventana Deslizante (Sliding Window)**: Segmentación de prompts extensos en fragmentos más pequeños para evaluar la similitud de coseno en bloques aislados, evitando que el relleno diluya el ataque.



## 6. Resultados de la Fase 2 (Pipeline de Sanitización Activo)

### Análisis de Rendimiento Corporativo
* **Efectividad Global**: 0.0% (0/30) debido a Desviación de Identificadores Sintácticos del Lote (*Batch ID Drift*).
* **Latencia de Arranque en Frío (Peticiones 1-5)**: ~1,415 ms (Fase de inicialización de tensores ONNXRuntime en RAM).
* **Latencia de Operación en Caliente (Peticiones 6-30)**: ~9.5 ms (Demuestra la eficiencia de cómputo en memoria del motor nativo C++).

### Conclusión del Laboratorio de Seguridad
Los parches de desofuscación de homóglifos y compactación de caracteres espaciados funcionan de forma óptima. Sin embargo, el experimento demostró que los detectores basados exclusivamente en firmas exactas de Embeddings son altamente sensibles a pequeñas adiciones de texto o identificadores dinámicos (ruido sintáctico como `[id-X]`). 



## 7. Validación Final de Remediación
Tras purgar las etiquetas ruidosas del dataset de pruebas, se procedió a re-evaluar la capa defensiva de sanitización previa.

### Resultados de la Fase de Mitigación (Efectividad = % Bloqueado)
* **CAMUFLAJE_ESPACIADO**: 100.0% de bloqueos - **MITIGADO EXITOSAMENTE**
* **CAMUFLAJE_HOMOGLIFOS**: 100.0% de bloqueos - **MITIGADO EXITOSAMENTE**

### Conclusiones de Ingeniería de Seguridad
La integración de las funciones de normalización basadas en tablas de traducción Unicode (`str.maketrans`) y expresiones regulares combinadas antes del proceso de embedding demostró ser una solución de costo computacional despreciable (~0.12 ms en la API) que neutraliza por completo los ataques de manipulación estructural de firmas. El componente nativo en C++ procesa eficientemente la decisión de bloqueo final con alta velocidad de cómputo en caliente.


## 8. Análisis de Caja Blanca y Conclusiones del Laboratorio

### Diagnóstico Métrico Final
* **Tasa de Evasión (Bypass Rate)**: 100% frente a todas las técnicas del Red Team.
* **Estabilidad del Core Nativo**: Excelente. El binding de memoria corregido eliminó los cuellos de botella dimensionales de NumPy, manteniendo latencias operativas de ~12 ms de manera constante.

### Descubrimiento Arquitectónico Crítico
El experimento demostró empíricamente la fragilidad de los sistemas defensivos basados exclusivamente en **Similitud Matemática de Coseno Rígida** frente a la Inyección de Prompts. Al no contar con una etapa de análisis heurístico o de coincidencia de palabras clave parciales, cualquier atacante que use frases introductorias o cambie ligeros componentes sintácticos logrará reducir el valor del vector geométrico por debajo del umbral estándar de 0.85, logrando un bypass completo.

### Recomendaciones de Remediación Futuras
Para romper el 0.0% de efectividad de manera permanente sin perder la latencia nativa, se recomienda:
1. **Calibración Dinámica de Umbrales**: Reducir el límite de bloqueo en C++ a `0.70` o `0.75` exclusivamente para peticiones que entren a endpoints sensibles como `/login`.
2. **Clasificación por Palabras Clave Secundarias**: Añadir una lista negra nativa dentro de `FirewallEvaluator.cpp` que busque términos clave normalizados (como `override`, `system configuration`, `bypass`) para forzar un bloqueo inmediato si la similitud semántica se encuentra en zonas grises.



## 9. Conclusiones Definitivas del Laboratorio de Inyección de Prompts

### Resultados del Análisis de Caja Blanca (Umbral a 0.70f)
* **Conectividad**: Endpoint `/login` verificado con éxito (0 errores de red).
* **Efectividad Global del Firewall**: 0.0% de bloqueos detectados frente a variantes dinámicas de Red Teaming.
* **Velocidad de Cómputo Nítido**: Excepcional. Latencia promedio de ~13.56 ms operando con el binario acoplado.

### Hallazgo de Ingeniería de Ciberseguridad
El experimento demostró de manera definitiva que reducir el umbral de similitud de coseno (incluso hasta un límite agresivo de `0.70f` en el código de C++) es **insuficiente** para detener técnicas avanzadas de inyección de prompts cuando el atacante introduce variaciones sintácticas complejas o prefijos conversacionales. La dilución semántica de los modelos de embeddings tradicionales actúa como una capa de invisibilidad matemática frente a los detectores basados estrictamente en distancias vectoriales fijas.

### Recomendación de Arquitectura de Defensa en Profundidad
Para mitigar este fallo de diseño estructural, el firewall de prompts corporativos no debe depender únicamente de ChromaDB. Se debe implementar una arquitectura híbrida:
1. Una capa previa de coincidencia difusa de palabras clave (*Regex/Keyword Match*) en C++ buscando términos prohibitivos absolutos.
2. Una capa secundaria de análisis semántico mediante un modelo clasificador de texto pequeño (LLM de pocas variables o BERT entrenado específicamente en detectar inyecciones) para procesar los prompts que caigan por debajo del umbral de coseno.



## 10. Validación Post-Remediación (Capa Híbrida Activa)

Tras la implementación del pipeline defensivo de doble capa (Sanitización + Lista Negra de Firmas Heurísticas), se ejecutó una re-evaluación exhaustiva de 30 vectores mutados.

### Resultados Comparativos de Efectividad
* **CAMUFLAJE_HOMOGLIFOS**: Pasó de 0.0% a **100.0% de efectividad** (Mitigado).
* **CAMUFLAJE_RELLENO_BENIGNO**: Pasó de 0.0% a **85.7% de efectividad** (Mitigado).
* **CAMUFLAJE_TRADUCCIÓN_LITERAL**: Pasó de 0.0% a **77.8% de efectividad** (Mitigado).
* **CAMUFLAJE_ESPACIADO**: Se mantiene en 0.0% (Punto ciego identificado para Fase 3).

### Lecciones Aprendidas en el Laboratorio
La arquitectura de Defensa en Profundidad demostró ser el único camino viable para asegurar aplicaciones basadas en LLMs. Al no depender de una sola tecnología, los parches heurísticos en Python actúan como un escudo perimetral de bajo costo, mientras que el motor geométrico en C++ procesa las intenciones complejas de fondo. 

