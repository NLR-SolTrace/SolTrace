---
title: "Motores de simulación"
---

Elija el motor que trazará la escena actual. Legacy Runner es la implementación de CPU de mayor compatibilidad. Embree Runner utiliza Intel Embree para acelerar el trazado en la CPU y normalmente es la opción de CPU preferida. NVIDIA GPU Runner utiliza OptiX para acelerar el trazado en una GPU NVIDIA compatible.

Las opciones disponibles dependen de cómo se compiló SolTrace. En particular, NVIDIA GPU Runner solo aparece en compilaciones que incluyen compatibilidad con OptiX y requiere una GPU NVIDIA y un controlador compatibles.

Independientemente del motor, use menos rayos para comprobaciones rápidas de configuración y aumente la cantidad cuando la geometría y los materiales estén listos para el análisis. Una mayor cantidad de rayos generalmente mejora la resolución estadística, pero requiere más tiempo y produce más datos de resultados.
