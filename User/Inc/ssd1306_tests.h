#ifndef SSD1306_TESTS_H
#define SSD1306_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

void ssd1306_TestAll(void);
void ssd1306_TestBorder(void);
void ssd1306_TestFonts1(void);
void ssd1306_TestFonts2(void);
void ssd1306_TestFonts3(void);
void ssd1306_TestFPS(void);
void ssd1306_TestLine(void);
void ssd1306_TestRectangle(void);
void ssd1306_TestRectangleFill(void);
void ssd1306_TestRectangleInvert(void);
void ssd1306_TestCircle(void);
void ssd1306_TestArc(void);
void ssd1306_TestPolyline(void);
void ssd1306_TestDrawBitmap(void);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_TESTS_H
