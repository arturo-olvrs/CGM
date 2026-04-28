#include <GLApp.h>
#include <FontRenderer.h>

Vec3 hsvToRgb(Vec3 hsv) {
  double h = hsv[0]; // Hue
  double s = hsv[1]; // Saturation
  double v = hsv[2]; // Value

  Vec3 rgb;

  // Saturation 0 means a shade of gray
  if (s <= 0.0) {
      rgb.r = rgb.g = rgb.b = v;
      return rgb;
  }

  // Normalization of Hue to 60-degree sectors
  if (h >= 360.0) h = 0.0;
  h /= 60.0;

  int i = static_cast<int>(std::floor(h));  // Sector index
  double f = h - i;   // Fractional part of hue sector

  // Intermediate values for RGB calculation
  double p = v * (1.0 - s);
  double q = v * (1.0 - (s * f));
  double t = v * (1.0 - (s * (1.0 - f)));

  // Mapping of sectors to RGB channels
  switch (i) {
      case 0: rgb.r = v; rgb.g = t; rgb.b = p; break;
      case 1: rgb.r = q; rgb.g = v; rgb.b = p; break;
      case 2: rgb.r = p; rgb.g = v; rgb.b = t; break;
      case 3: rgb.r = p; rgb.g = q; rgb.b = v; break;
      case 4: rgb.r = t; rgb.g = p; rgb.b = v; break;
      default: rgb.r = v; rgb.g = p; rgb.b = q; break;
  }

  return rgb;
}

class MyGLApp : public GLApp {
public:
  Image image{640,480};
  FontRenderer fr{"helvetica_neue.bmp", "helvetica_neue.pos"};
  std::shared_ptr<FontEngine> fe{nullptr};
  std::string text;

  MyGLApp() : GLApp{800,800,1,"Color Picker"} {}

  Vec3 convertPosFromHSVToRGB(float h, float s) {
    
    return hsvToRgb(Vec3{h*360.0f, s, 1.0f});
  }
  
  virtual void init() override {
    fe = fr.generateFontEngine();
    for (uint32_t y = 0;y<image.height;++y) {
      for (uint32_t x = 0;x<image.width;++x) {
        const Vec3 rgb = convertPosFromHSVToRGB(float(x)/image.width, float(y)/image.height);
        image.setNormalizedValue(x,y,0,rgb.r); image.setNormalizedValue(x,y,1,rgb.g);
        image.setNormalizedValue(x,y,2,rgb.b); image.setValue(x,y,3,255);
      }
    }
  }
  
  virtual void mouseMove(double xPosition, double yPosition) override {
    Dimensions s = glEnv.getWindowSize();
    if (xPosition < 0 || xPosition > s.width || yPosition < 0 || yPosition > s.height) return;
    const Vec3 hsv{float(360*xPosition/s.width),float(1.0-yPosition/s.height),1.0f};
    const Vec3 rgb = convertPosFromHSVToRGB(float(xPosition/s.width), float(1.0-yPosition/s.height));
    std::stringstream ss; ss << "HSV: " << hsv << "  RGB: " << rgb; text = ss.str();
  }
    
  virtual void draw() override {
    drawImage(image);

    const Dimensions dim{ glEnv.getFramebufferSize() };
    fe->render(text, dim.aspect(), 0.03f, {0,-0.9f}, Alignment::Center, {0,0,0,1});
  }
} myApp;

#ifdef _WIN32
#include <Windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
  try {
    myApp.run();
  }
  catch (const GLException& e) {
    std::stringstream ss;
    ss << "Insufficient OpenGL Support " << e.what();
#ifndef _WIN32
    std::cerr << ss.str().c_str() << std::endl;
#else
    MessageBoxA(
      NULL,
      ss.str().c_str(),
      "OpenGL Error",
      MB_ICONERROR | MB_OK
    );
#endif
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
