import { clsx } from '@cfx-dev/ui-components';

import s from './ServerTitle.module.scss';

export interface ServerTitleProps {
  title: string;
  size?: 'small' | 'normal' | 'large' | 'xlarge' | 'xxlarge' | 'xxxlarge';
  truncated?: boolean;
}

export function ServerTitle(props: ServerTitleProps) {
  const {
    title,
    size = 'normal',
    truncated = false,
  } = props;

  const rootClassName = clsx(s.root, s[`size-${size}`], {
    [s.truncated]: truncated,
  });

  return (
    <span className={rootClassName}>{title}</span>
  );
}
