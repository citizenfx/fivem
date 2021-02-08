import * as React from 'react';
import classnames from 'classnames';
import s from './Stepper.module.scss';
import { BsCheckAll, BsDot } from 'react-icons/bs';
import { Indicator } from 'components/Indicator/Indicator';

export interface StepperProps {
  step: number,
  steps: Record<number, React.ReactNode>,
  className?: string,
}

const getIcon = (active: boolean, passed: boolean): React.ReactNode => {
  if (active) {
    return <Indicator />;
  }

  if (passed) {
    return <BsCheckAll />;
  }

  return <BsDot />;
};

export const Stepper = React.memo(function Stepper(props: StepperProps) {
  const { step, steps, className } = props;

  const rootClassName = classnames(s.root, className);

  const nodes = Object.entries(steps).map(([index, label]) => {
    const idx = parseInt(index, 10);

    const active = step === idx;
    const passed = step > idx;

    const icon = getIcon(active, passed);

    const stepClassName = classnames(s.step, {
      [s.active]: active,
      [s.passed]: passed,
    });

    return (
      <div key={`${index}-${label}`} className={stepClassName}>
        <div className={s.icon}>
          {icon}
        </div>

        <div className={s.label}>
          {label}
        </div>
      </div>
    );
  });

  return (
    <div className={rootClassName}>
      {nodes}
    </div>
  );
});
