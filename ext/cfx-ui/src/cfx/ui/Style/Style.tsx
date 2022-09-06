import React from "react";

const StyleContext = React.createContext<React.CSSProperties>({});

export function useContextualStyle() {
  return React.useContext(StyleContext);
}

export type StyleProps = React.CSSProperties & {
  children: React.ReactNode,
};
export function Style(props: StyleProps) {
  const { children, ...style } = props;

  return (
    <StyleContext.Provider value={style}>
      {children}
    </StyleContext.Provider>
  );
}

const EMPTY = {};

Style.Reset = ({ children }) => (
  <StyleContext.Provider value={EMPTY}>
    {children}
  </StyleContext.Provider>
);
