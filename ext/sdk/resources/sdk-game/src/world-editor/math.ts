export const RADS_IN_DEG = 0.0174533; // Math.PI / 180

export function clamp(num: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, num));
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

export class Vec3 {
  static zero(): Vec3 {
    return new Vec3(0, 0, 0);
  }

  constructor(
    public x: number,
    public y: number,
    public z: number,
  ) {}

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
    const inverseLength = Q_rsqrt(this.x*this.x + this.y*this.y + this.z*this.z);

    return new Vec3(
      this.x * inverseLength,
      this.y * inverseLength,
      this.z * inverseLength,
    )
  }

  copy(): Vec3 {
    return new Vec3(this.x, this.y, this.z);
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
  ) {}

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
  return 1/Q_rsqrt(x**2 + y**2 + z**2 + w**2);
}

function normalize([x, y, z, w]: number[]): [number, number, number, number] {
  const inverseLength = Q_rsqrt(x**2 + y**2 + z**2 + w**2);

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

export function applyEntityMatrix(entity: number, mat: Float32Array) {
  SetEntityMatrix(
    entity,
    mat[4], mat[5], mat[6], // r
    mat[0], mat[1], mat[2], // f
    mat[8], mat[9], mat[10], // u
    mat[12], mat[13], mat[14], // a
  );
}

export function toPrecision(n: number, precision: number): number {
  return (n * precision | 0) / precision;
}

export function limitPrecision(data: number[], precision: number): number[] {
  return data.map((n) => toPrecision(n, precision));
}
