import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './PremiumBadge.module.scss';

export const VALID_PREMIUM_LEVELS = ['pt', 'au', 'ag'];

export type PremiumBadgeLevel =
  | 'pt'
  | 'au'
  | 'ag'
  | string;

export interface PremiumBadgeProps {
  level: PremiumBadgeLevel,
  className?: string,
}

export const PremiumBadge = React.forwardRef((props: PremiumBadgeProps, ref: React.Ref<HTMLDivElement>) => {
  const {
    level,
    className,
  } = props;

  if (!VALID_PREMIUM_LEVELS.includes(level)) {
    return null;
  }

  const rootClassName = clsx(s.root, className, s[`level-${level}`]);

  return (
    <div ref={ref} className={rootClassName}>
      <span>
        {level}
      </span>
    </div>
  );
});
