import { LinkButton as UiLinkButton } from '@cfx-dev/ui-components';
import React from 'react';
import { Link } from 'react-router-dom';

export type LinkButtonProps = React.ComponentProps<typeof UiLinkButton>;

export const LinkButton = React.forwardRef(function LinkButton(
  props: LinkButtonProps,
  ref: React.Ref<HTMLAnchorElement>,
) {
  return (
    <UiLinkButton
      ref={ref}
      Component={Link}
      {...props}
    />
  );
});
