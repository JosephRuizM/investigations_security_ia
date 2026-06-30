import os
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

# Rutas hacia ambos archivos fuentes de C++
fuente_evaluador = os.path.join("evaluador_firewall", "evaluador_firewall", "FirewallEvaluator.cpp")
fuente_bindings = os.path.join("evaluador_firewall", "evaluador_firewall", "bindings.cpp")

ext_modules = [
    Pybind11Extension(
        "motor_firewall",
        sources=[fuente_evaluador, fuente_bindings], # Enlaza ambos archivos juntos
        include_dirs=[os.path.join("evaluador_firewall", "evaluador_firewall")],
        extra_compile_args=["/O2", "/std:c++17"]
    ),
]

setup(
    name="motor_firewall",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
