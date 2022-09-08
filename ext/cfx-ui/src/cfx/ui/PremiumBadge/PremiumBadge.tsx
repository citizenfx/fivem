import React from "react";
import { Title } from "../Title/Title";

const map = Object.freeze({
  ag: {
    emoji: '💿',
    title: 'Element Club Argentum',
  },
  au: {
    emoji: '📀',
    title: 'Element Club Aurum',
  },
  pt: {
    emoji: '🌟',
    title: 'Element Club Platinum',
  },
});

export type PremiumBadgeLevel =
  | keyof typeof map
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

  if (!map[level]) {
    return null;
  }

  return (
    <div ref={ref} className={className}>
      <Title title={map[level].title}>
        <span>
          {map[level].emoji}
        </span>
      </Title>
    </div>
  );
});
