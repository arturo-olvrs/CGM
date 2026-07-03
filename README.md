# Computer Graphics (CGM) — Universität Duisburg-Essen 🇩🇪

Este repositorio contiene las prácticas, proyectos y algoritmos desarrollados durante mi estancia **Erasmus** en la **Universität Duisburg-Essen (UDE)**, correspondientes a la asignatura de nivel avanzado **Computer Graphics**. 

Para ver los enunciados de los ejericios, vistar [esta web](https://www.cgvis.de/teaching/st2026/CGM/).


El curso se enfoca en el estudio e implementación de técnicas de renderizado global (Global Illumination), física de la luz, gráficos volumétricos y optimización de imágenes de alto rendimiento.

## 🚀 Tecnologías y Herramientas
*   **Lenguaje/Framework Principal:** C++ / OpenGL
*   **Matemáticas Gráficas:** GLM (OpenGL Mathematics)
*   **Gestión de Ventanas y Contexto:** GLFW / GLEW

---

## 📚 Contenido del Curso y Características Implementadas

El repositorio está dividido en los bloques clave de la asignatura:

### 1. Global Illumination & Ray-Based Rendering
*   **Ray Tracing Review:** Estructuras de aceleración y cálculo de intersecciones.
*   **Path Tracing:** Simulación de transporte de luz realista mediante métodos de Monte Carlo.
*   **Photon Mapping:** Renderizado eficiente de cáusticas y efectos de iluminación compleja.
*   **Radiosity:** Cálculo de iluminación difusa entre superficies mediante factores de forma.

### 2. Advanced Light & Material Simulation
*   **Radiance & Precomputed Radiance Transfer (PRT):** Simulación de iluminación realista en tiempo real mediante el cálculo previo de la transferencia de luz.
*   **Subsurface Scattering (SSS):** Simulación de materiales translúcidos (piel, mármol, cera) donde la luz penetra la superficie.
*   **Irradiance Volumes & Ambient Occlusion:** Técnicas para aproximar sombras de contacto e iluminación global indirecta.
*   **Sampling:** Técnicas avanzadas de muestreo estadístico para la reducción de ruido en renderizado de sombreado.

### 3. Volume Graphics & Display Technologies
*   **Volume Rendering:** Técnicas de visualización de datos volumétricos (como simulaciones fluidas o datos médicos).
*   **High Dynamic Range Imaging (HDRI):** Mapeo de tonos (tonemapping) y gestión de amplio rango dinámico.
*   **Image Compression:** Algoritmos aplicados a la optimización de texturas y buffers.
*   **Virtual Reality (VR):** Conceptos e implementación de distorsión de lente, renderizado estéreo y latencia en entornos inmersivos.
