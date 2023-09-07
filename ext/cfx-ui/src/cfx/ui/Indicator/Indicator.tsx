import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Indicator.module.scss';

const delay1 = { animationDelay: '0ms' };
const delay2 = { animationDelay: '50ms' };
const delay3 = { animationDelay: '100ms' };
const delay4 = { animationDelay: '150ms' };

export interface IndicatorProps {
  className?: string,
}

export const Indicator = React.memo(function Indicator({ className }: IndicatorProps) {
  return (
    <div className={clsx(s.root, className)}>
      <div className={s["lds-dual-ring"]}></div>
      {/* <div style={delay1} />
      <div style={delay2} />
      <div style={delay3} />
      <div style={delay4} /> */}
    </div>
  );
});
