import React from "react";
import { attachOutlet } from "cfx/utils/outlet";

const BackdropOutlet = attachOutlet('backdrop');

export interface BackdropPortalProps {
  children?: React.ReactNode,
}

export function BackdropPortal({ children }: BackdropPortalProps) {
  return (
    <BackdropOutlet>
      {children}
    </BackdropOutlet>
  );
}
