import { WEEntityMatrix, WEEntityMatrixIndex } from "./map-types";

export const RADS_IN_DEG = Math.PI / 180
export const DEGS_IN_RAD = 180 / Math.PI;

export function clamp(num: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, num));
}

export function rad2deg(rad: number): number {
  return rad * DEGS_IN_RAD;
}

export function deg2rad(deg: number): number {
  return deg * RADS_IN_DEG;
}

const fisrAB = new ArrayBuffer(4);
const fisrF32 = new Float32Array(fisrAB);
const fisrU32 = new Uint32Array(fisrAB);
const threehalfs = 1.5;
export function Q_rsqrt(number: number): number {
  let i;
  let y;

  const x2 = number * 0.5;

  y = number;
  //evil floating bit level hacking
  // var buf = new ArrayBuffer(4);

  fisrF32[0] = number;
  // (new Float32Array(buf))[0] = number;

  i = fisrU32[0];
  // i =  (new Uint32Array(buf))[0];

  i = (0x5f3759df - (i >> 1)); //What the fuck?

  fisrU32[0] = i;
  // (new Uint32Array(buf))[0] = i;

  y = fisrF32[0];
  // y = (new Float32Array(buf))[0];

  y = y * (threehalfs - (x2 * y * y));   // 1st iteration
  y = y * (threehalfs - (x2 * y * y));   // 2nd iteration, this can be removed

  return y;
}

export class Vec3 {
  static zero(): Vec3 {
    return new Vec3(0, 0, 0);
  }

  constructor(
    public x: number,
    public y: number,
    public z: number,
  ) { }

  add(vec: Vec3): Vec3 {
    this.x += vec.x;
    this.y += vec.y;
    this.z += vec.z;

    return this;
  }

  mult(mag: number | Vec3): Vec3 {
    if (typeof mag === 'number') {
      this.x *= mag;
      this.y *= mag;
      this.z *= mag;

      return this;
    }

    this.x *= mag.x;
    this.y *= mag.y;
    this.z *= mag.z;

    return this;
  }

  normal(): Vec3 {
    const inverseLength = Q_rsqrt(this.x * this.x + this.y * this.y + this.z * this.z);

    return new Vec3(
      this.x * inverseLength,
      this.y * inverseLength,
      this.z * inverseLength,
    )
  }

  length(): number {
    return vectorLength([
      this.x,
      this.y,
      this.z,
    ]);
  }

  dot(vec: Vec3): number {
    return this.x*vec.x + this.y*vec.y + this.z*vec.z;
  }

  copy(): Vec3 {
    return new Vec3(this.x, this.y, this.z);
  }

  cross(vec: Vec3): Vec3 {
    return crossProduct(this, vec);
  }

  toArray(): [number, number, number] {
    return [this.x, this.y, this.z];
  }
}

export class RotDeg3 {
  static zero(): RotDeg3 {
    return new RotDeg3(0, 0, 0);
  }

  constructor(
    public x: number,
    public y: number,
    public z: number,
  ) { }

  forward(): Vec3 {
    const radX = this.x * RADS_IN_DEG;
    const radY = this.y * RADS_IN_DEG;
    const radZ = this.z * RADS_IN_DEG;

    const sinX = Math.sin(radX);
    const sinY = Math.sin(radY);
    const sinZ = Math.sin(radZ);

    const cosX = Math.cos(radX);
    const cosY = Math.cos(radY);
    const cosZ = Math.cos(radZ);

    return new Vec3(
      cosZ * sinX * sinY - cosX * sinZ,
      cosX * cosZ - sinX * sinY * sinZ,
      cosY * sinX,
    ).normal();
  }

  left(): Vec3 {
    const radX = this.x * RADS_IN_DEG;
    const radY = this.y * RADS_IN_DEG;
    const radZ = this.z * RADS_IN_DEG;

    const sinX = Math.sin(radX);
    const sinY = Math.sin(radY);
    const sinZ = Math.sin(radZ);

    const cosX = Math.cos(radX);
    const cosY = Math.cos(radY);
    const cosZ = Math.cos(radZ);

    return new Vec3(
      cosY * cosZ,
      cosY * sinZ,
      -sinY,
    ).normal();
  }

  directions(): [Vec3, Vec3] {
    const radX = this.x * RADS_IN_DEG;
    const radY = this.y * RADS_IN_DEG;
    const radZ = this.z * RADS_IN_DEG;

    const sinX = Math.sin(radX);
    const sinY = Math.sin(radY);
    const sinZ = Math.sin(radZ);

    const cosX = Math.cos(radX);
    const cosY = Math.cos(radY);
    const cosZ = Math.cos(radZ);

    return [
      new Vec3(
        cosZ * sinX * sinY - cosX * sinZ,
        cosX * cosZ - sinX * sinY * sinZ,
        cosY * sinX,
      ).normal(),
      new Vec3(
        cosY * cosZ,
        cosY * sinZ,
        -sinY,
      ).normal(),
    ];
  }

  clamp(): RotDeg3 {
    this.x = clamp(this.x, -90, 90);
    this.y = this.y % 360;
    this.z = this.z % 360;

    return this;
  }
}

export function getScale(mat: Float32Array): [number, number, number] {
  return [
    vectorLength([mat[0], mat[1], mat[2], mat[3]]),
    vectorLength([mat[4], mat[5], mat[6], mat[7]]),
    vectorLength([mat[8], mat[9], mat[10], mat[11]]),
  ];
}

export function applyScale(mat: Float32Array, scale: [number, number, number]) {
  const nscale = getScale(mat);
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
  return 1 / Q_rsqrt(x ** 2 + y ** 2 + z ** 2 + w ** 2);
}

function normalize([x, y, z, w]: number[]): [number, number, number, number] {
  const inverseLength = Q_rsqrt(x ** 2 + y ** 2 + z ** 2 + w ** 2);

  return [
    x * inverseLength,
    y * inverseLength,
    z * inverseLength,
    w * inverseLength,
  ];
}

export function makeEntityMatrix(entity: number): Float32Array {
  const [f, r, u, a] = GetEntityMatrix(entity);

  return new Float32Array([
    r[0], r[1], r[2], 0,
    f[0], f[1], f[2], 0,
    u[0], u[1], u[2], 0,
    a[0], a[1], a[2], 1,
  ]);
}

export function applyEntityMatrix(entity: number, mat: Float32Array | number[]) {
  SetEntityMatrix(
    entity,
    mat[4], mat[5], mat[6], // right
    mat[0], mat[1], mat[2], // forward
    mat[8], mat[9], mat[10], // up
    mat[12], mat[13], mat[14], // at
  );
}

export function applyAdditionMatrix(entity: number, mat: WEEntityMatrix | number[]) {
  SetEntityCoords(
    entity,
    mat[WEEntityMatrixIndex.AX], mat[WEEntityMatrixIndex.AY], mat[WEEntityMatrixIndex.AZ],
    false, false, false, false,
  );

  const rot = eulerFromMatrix(mat);

  SetEntityRotation(entity, rot[0], rot[1], rot[2], 2, false);

  SetEntityMatrix(
    entity,
    mat[4], mat[5], mat[6], // right
    mat[0], mat[1], mat[2], // forward
    mat[8], mat[9], mat[10], // up
    mat[12], mat[13], mat[14], // at
  );
}

export function toPrecision(n: number, precision: number): number {
  return (n * precision | 0) / precision;
}

export function limitPrecision(data: number[], precision: number): number[] {
  return data.map((n) => toPrecision(n, precision));
}

const RAD_TO_DEG = 180 / Math.PI;
const DEG_TO_RAD = Math.PI / 180;

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

export function rotationBetween(vec1: Vec3, vec2: Vec3): RotDeg3 {
  const z = Math.atan2(vec2.y - vec1.y, vec2.x - vec1.x);
  const x = 0;
  const y = Math.atan2(vec2.z - vec1.z, vec2.x - vec1.x);

  return new RotDeg3(rad2deg(x), rad2deg(y), rad2deg(z));
}

export function angleBetween([ox, oy], [x, y]) {
  let a = Math.atan2(oy, ox) - Math.atan2(y, x);
  if (a < 0) {
    a += 2 * Math.PI;
  }

  return rad2deg(a);
}

export function rotation([ox, oy], [x, y]) {
  return -angleBetween(
    [0, 1],
    [x - ox, y - oy],
  );
}

export function crossProduct(a: Vec3, b: Vec3): Vec3 {
  return new Vec3(
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
  );
}
