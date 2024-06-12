import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './Avatar.module.scss';

const placeholder = new URL('assets/images/avatar-placeholder.jpg', import.meta.url).toString();

export interface AvatarProps {
  url?: string | undefined | null;
  size?: 'none' | 'small' | 'normal' | 'large' | 'xxxlarge';

  className?: string;
}

export const Avatar = React.forwardRef(function Avatar(props: AvatarProps, ref: React.Ref<HTMLDivElement>) {
  const {
    url,
    size = 'normal',
    className,
  } = props;

  const rootClassName = clsx(s.root, className, s[`size-${size}`]);

  const style: React.CSSProperties = {
    backgroundImage: `url(${url || placeholder})`,
  };

  return (
    <div ref={ref} style={style} className={rootClassName} />
  );
});
