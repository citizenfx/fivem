import { clsx } from 'cfx/utils/clsx';

import s from './Spacer.module.scss';

export interface SpacerProps {
  size?: 'none' | 'thin' | 'small' | 'normal' | 'large' | 'xlarge';
  vertical?: boolean;
}

export function Spacer(props: SpacerProps) {
  const {
    size = 'normal',
    vertical = false,
  } = props;

  const rootClassName = clsx(s.root, s[`size-${size}`], {
    [s.horizontal]: !vertical,
    [s.vertical]: vertical,
  });

  return (
    <div className={rootClassName} />
  );
}
