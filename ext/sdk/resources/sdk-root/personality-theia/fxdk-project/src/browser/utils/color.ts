import { joaat } from "./joaat";

// https://stackoverflow.com/a/9493060
function hue2rgb(v1: number, v2: number, vH: number): number {
  if(vH < 0) vH += 1;
  if(vH > 1) vH -= 1;
  if(vH < 1/6) return v1 + (v2 - v1) * 6 * vH;
  if(vH < 1/2) return v2;
  if(vH < 2/3) return v1 + (v2 - v1) * (2/3 - vH) * 6;
  return v1;
}

export function hslToRgb(h: number, s: number, l: number): [number, number, number] {
  var r, g, b;

  if (s == 0) {
      r = g = b = l; // achromatic
  } else {
      const v2 = l < 0.5
        ? l * (1 + s)
        : l + s - l * s;

      const v1 = 2 * l - v2;

      r = 255 * hue2rgb(v1, v2, h + 1/3);
      g = 255 * hue2rgb(v1, v2, h);
      b = 255 * hue2rgb(v1, v2, h - 1/3);
  }

  return [r|0, g|0, b|0];
}

export function hslForKey(key: string): [number, number, number] {
  return [joaat(key) % 360, 80, 40];
}

export function rgbForKey(key: string): [number, number, number] {
  const hue = (joaat(key) % 360) / 360;

  return hslToRgb(hue, .8, .4);
}

const colorizeColorIndexRegexp = /\^([0-9])/g;
const colorizeRemnantsRegexp = /<span[^>]*><\/span[^>]*>/g;
export function colorizeString(msg: string): string {
  const s = "<span>" + (msg.replace(colorizeColorIndexRegexp, (str, color) => `</span><span class="color-${color}">`)) + "</span>";

  return s.replace(colorizeRemnantsRegexp, '');
}
