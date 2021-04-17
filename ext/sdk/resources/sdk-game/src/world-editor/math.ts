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
