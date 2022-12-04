import React from "react";

export function nl2br(text: string): string {
  return text.split('\n').join('<br>');
}

export function nl2brx(text: string): React.ReactNode {
  const parts = text.split('\n');

  return parts.map((line, i) => {
    if (i === parts.length - 1) {
      return line;
    }

    return (
      <React.Fragment key={i + line}>
        {line}
        <br/>
      </React.Fragment>
    );
  });
}
