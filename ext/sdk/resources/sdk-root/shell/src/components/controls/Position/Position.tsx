import React from 'react';
import s from './Position.module.scss';
import { PositionInput } from './PositionInput';

export interface PositionProps {
  x: number,
  y: number,
  z: number,

  onChange?(x: number, y: number, z: number): void,
}

function limitPrecision(n: number): number {
  return (n * 1000 | 0) / 1000;
}

export function Position(props: PositionProps) {
  const { x, y, z, onChange } = props;

  const handleChangeX = (x: number) => onChange(limitPrecision(x), y, z);
  const handleChangeY = (y: number) => onChange(x, limitPrecision(y), z);
  const handleChangeZ = (z: number) => onChange(x, y, limitPrecision(z));

  return (
    <div className={s.root}>
      <div className={s.inputs}>
        <PositionInput
          label="x:"
          value={x}
          onChange={handleChangeX}
        />
        <PositionInput
          label="y:"
          value={y}
          onChange={handleChangeY}
        />
        <PositionInput
          label="z:"
          value={z}
          onChange={handleChangeZ}
        />
      </div>
    </div>
  );
}
