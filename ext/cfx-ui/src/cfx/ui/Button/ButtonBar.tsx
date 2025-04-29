import { clsx } from 'cfx/utils/clsx';
import s from './Button.module.scss';

export interface ButtonBarProps {
  children?: React.ReactNode,
  className?: string,
}

export function ButtonBar(props: ButtonBarProps) {
  const {
    children,
    className,
  } = props;

  return (
    <div className={clsx(s.bar, className)}>
      {children}
    </div>
  );
}
