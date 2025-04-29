import React from "react";
import { clsx } from "cfx/utils/clsx";
import 'flag-icons/css/flag-icons.min.css';
import s from './CountryFlag.module.scss';
import { useIntlService } from "cfx/common/services/intl/intl.service";
import { Title } from "cfx/ui/Title/Title";

export interface CountryFlagProps {
  country: string,
  locale?: string,
  className?: string,

  forceShow?: boolean,
}

export const CountryFlag = React.forwardRef((props: CountryFlagProps, ref: React.Ref<HTMLDivElement>) => {
  const {
    country,
    locale,
    className,
    forceShow = false,
  } = props;

  const IntlService = useIntlService();

  // #TODO: 001 snail?
  if (!forceShow && (country === '001' || country === 'AQ' || country === 'aq')) {
    return null;
  }

  const rootClassName = clsx(s.root, className, `fi-${country.toLowerCase()}`);

  return (
    <Title title={IntlService.defaultDisplayNames.of(locale ?? country)}>
      <div
        ref={ref}
        className={rootClassName}
      />
    </Title>
  );
});
