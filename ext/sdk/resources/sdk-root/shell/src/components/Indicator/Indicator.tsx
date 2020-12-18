import * as React from 'react';
import s from './Indicator.module.scss';

const delay1 = { animationDelay: '0ms' };
const delay2 = { animationDelay: '50ms' };
const delay3 = { animationDelay: '100ms' };
const delay4 = { animationDelay: '150ms' };

export const Indicator = React.memo(function Indicator() {
  return (
    <div className={s.root}>
      <div style={delay1} />
      <div style={delay2} />
      <div style={delay3} />
      <div style={delay4} />
    </div>
  );
});
