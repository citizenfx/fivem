import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import { Title } from '../Title/Title';

import s from './PremiumBadge.module.scss';

const map = Object.freeze({
  ag: {
    // https://github.com/microsoft/fluentui-emoji/tree/main/assets/Optical%20disk
    emoji: '💿',
    imgSrc: new URL('./assets/optical_disk_flat.svg', import.meta.url).toString(),
    title: 'Element Club Argentum',
  },
  au: {
    // https://github.com/microsoft/fluentui-emoji/tree/main/assets/Dvd
    emoji: '📀',
    imgSrc: new URL('./assets/dvd_flat.svg', import.meta.url).toString(),
    title: 'Element Club Aurum',
  },
  pt: {
    // https://github.com/microsoft/fluentui-emoji/tree/main/assets/Glowing%20star
    emoji: '🌟',
    imgSrc: new URL('./assets/glowing_star_flat.svg', import.meta.url).toString(),
    title: 'Element Club Platinum',
  },
});

export type PremiumBadgeLevel = keyof typeof map | string;

export interface PremiumBadgeProps {
  level: PremiumBadgeLevel;
  className?: string;
}

export const PremiumBadge = React.forwardRef(function PremiumBadge(
  props: PremiumBadgeProps,
  ref: React.Ref<HTMLDivElement>,
) {
  const {
    level,
    className,
  } = props;

  if (!map[level]) {
    return null;
  }

  const rootClassName = clsx(s.root, className);

  return (
    <div ref={ref} className={rootClassName}>
      <Title title={map[level].title}>
        <img src={map[level].imgSrc} alt={map[level].title} />
      </Title>
    </div>
  );
});
