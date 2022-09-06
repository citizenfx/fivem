import { clsx } from 'cfx/utils/clsx';
import React from 'react';
import s from './Avatar.module.scss';

export interface AvatarProps {
  url: string,
  size?: 'none' | 'small' | 'normal' | 'large',

  className?: string,
}

export const Avatar = React.forwardRef(function Avatar(props: AvatarProps, ref: React.Ref<HTMLDivElement>) {
  const {
    url,
    size = 'normal',
    className,
  } = props;

  const rootClassName = clsx(s.root, className, s[`size-${size}`]);

  const style: React.CSSProperties = {
    backgroundImage: `url(${url})`,
  };

  return (
    <div
      ref={ref}
      style={style}
      className={rootClassName}
    />
  );
});
