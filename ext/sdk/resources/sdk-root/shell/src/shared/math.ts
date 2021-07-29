import { WEEntityMatrix, WEEntityMatrixIndex } from "backend/world-editor/world-editor-types";

const RAD_TO_DEG = 180 / Math.PI;
const DEG_TO_RAD = Math.PI/ 180;

export function limitPrecision(n: number, precision: number = 1000): number {
  return Math.round(n * precision) / precision;
}

/**
 *
 * @param mat
 * @returns [number, number, number] ZXY order
 */
export function eulerFromMatrix(mat: WEEntityMatrix | number[]): [number, number, number] {
  let x = 0;
  let y = 0;
  let z = 0;

  const m11 = mat[WEEntityMatrixIndex.RX];
  const m12 = mat[WEEntityMatrixIndex.FX];
  const m13 = mat[WEEntityMatrixIndex.UX];

  const m21 = mat[WEEntityMatrixIndex.RY];
  const m22 = mat[WEEntityMatrixIndex.FY];
  const m23 = mat[WEEntityMatrixIndex.UY];

  const m31 = mat[WEEntityMatrixIndex.RZ];
  const m32 = mat[WEEntityMatrixIndex.FZ];
  const m33 = mat[WEEntityMatrixIndex.UZ];

  x = Math.asin(Math.max(-1, Math.min(1, m32)));

  if (Math.abs(m32) < 0.99999) {
    y = Math.atan2(-m31, m33);
    z = Math.atan2(-m12, m22);
  } else {
    y = 0;
    z = Math.atan2(m21, m11);
  }

  return [
    x * RAD_TO_DEG,
    y * RAD_TO_DEG,
    z * RAD_TO_DEG,
  ];
}

/**
 *
 * @param [number, number, number] ZXY order
 */
export function eulerToMatrix(euler: [number, number, number], toMat?: WEEntityMatrix): WEEntityMatrix {
  const x = euler[0] * DEG_TO_RAD;
  const y = euler[1] * DEG_TO_RAD;
  const z = euler[2] * DEG_TO_RAD;

  const mat = toMat || Array(16).fill(0) as WEEntityMatrix;

  const a = Math.cos(x);
  const b = Math.sin(x);
  const c = Math.cos(y);
  const d = Math.sin(y);
  const e = Math.cos(z);
  const f = Math.sin(z);

  const ce = c*e;
  const cf = c*f;
  const de = d*e;
  const df = d*f;

  mat[WEEntityMatrixIndex.RX] = ce - df * b;
  mat[WEEntityMatrixIndex.FX] = -a * f;
  mat[WEEntityMatrixIndex.UX] = de + cf * b;

  mat[WEEntityMatrixIndex.RY] = cf + de * b;
  mat[WEEntityMatrixIndex.FY] = a * e;
  mat[WEEntityMatrixIndex.UY] = df - ce * b;

  mat[WEEntityMatrixIndex.RZ] = -a * d;
  mat[WEEntityMatrixIndex.FZ] = b;
  mat[WEEntityMatrixIndex.UZ] = a * c;

  return mat;
}

const fisrAB = new ArrayBuffer(4);
const fisrF32 = new Float32Array(fisrAB);
const fisrU32 = new Uint32Array(fisrAB);
const threehalfs = 1.5;
export function Q_rsqrt(number: number): number
{
    let i;
    let y;

    const x2 = number * 0.5;

    y = number;
    //evil floating bit level hacking
    // var buf = new ArrayBuffer(4);

    fisrF32[0] = number;
    // (new Float32Array(buf))[0] = number;

    i =  fisrU32[0];
    // i =  (new Uint32Array(buf))[0];

    i = (0x5f3759df - (i >> 1)); //What the fuck?

    fisrU32[0] = i;
    // (new Uint32Array(buf))[0] = i;

    y = fisrF32[0];
    // y = (new Float32Array(buf))[0];

    y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
    y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}

export function getScale(mat: number[]): [number, number, number] {
  return [
    vectorLength([mat[0], mat[1], mat[2], mat[3]]),
    vectorLength([mat[4], mat[5], mat[6], mat[7]]),
    vectorLength([mat[8], mat[9], mat[10], mat[11]]),
  ];
}

export function applyScale(mat: number[], scale: [number, number, number], originalScale?: [number, number, number]) {
  const nscale = originalScale || getScale(mat);
  const [x, y, z] = [scale[0] / nscale[0], scale[1] / nscale[1], scale[2] / nscale[2]]

  mat[0] *= x;
  mat[1] *= x;
  mat[2] *= x;
  mat[3] *= x;

  mat[4] *= y;
  mat[5] *= y;
  mat[6] *= y;
  mat[7] *= y;

  mat[8] *= z;
  mat[9] *= z;
  mat[10] *= z;
  mat[11] *= z;
}

export function vectorLength([x, y, z = 0, w = 0]: number[]): number {
  return 1/Q_rsqrt(x**2 + y**2 + z**2 + w**2);
}

const SCALE_1: [number, number, number] = [1, 1, 1];

export function applyRotation(mat: WEEntityMatrix, euler: [number, number, number]) {
  const scale = getScale(mat);

  const rotatedMat = Array.from(mat) as WEEntityMatrix;
  applyScale(rotatedMat, SCALE_1, scale);

  eulerToMatrix(euler, rotatedMat);

  applyScale(rotatedMat, scale, SCALE_1);

  mat[WEEntityMatrixIndex.RX] = rotatedMat[WEEntityMatrixIndex.RX];
  mat[WEEntityMatrixIndex.RY] = rotatedMat[WEEntityMatrixIndex.RY];
  mat[WEEntityMatrixIndex.RZ] = rotatedMat[WEEntityMatrixIndex.RZ];

  mat[WEEntityMatrixIndex.FX] = rotatedMat[WEEntityMatrixIndex.FX];
  mat[WEEntityMatrixIndex.FY] = rotatedMat[WEEntityMatrixIndex.FY];
  mat[WEEntityMatrixIndex.FZ] = rotatedMat[WEEntityMatrixIndex.FZ];

  mat[WEEntityMatrixIndex.UX] = rotatedMat[WEEntityMatrixIndex.UX];
  mat[WEEntityMatrixIndex.UY] = rotatedMat[WEEntityMatrixIndex.UY];
  mat[WEEntityMatrixIndex.UZ] = rotatedMat[WEEntityMatrixIndex.UZ];
}

export function clamp01(n: number): number {
  if (n < 0) {
    return 0;
  }

  if (n > 1) {
    return 1;
  }

  return n;
}

export function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}
