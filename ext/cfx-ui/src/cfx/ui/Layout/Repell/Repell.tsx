import React from 'react';
import { Flex, FlexProps } from '../Flex/Flex';

export type RepellProps = Omit<FlexProps, 'repell'>;

export const Repell = React.forwardRef(function Repell(props: RepellProps, ref: React.Ref<HTMLDivElement>) {
  const {
    children,
    ...flexProps
  } = props;

  return (
    <Flex
      ref={ref}
      repell
      {...flexProps}
    >
      {children}
    </Flex>
  );
});
