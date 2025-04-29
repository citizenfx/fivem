import React from 'react';
import './GlobalCloseButton.scss';
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { Icons } from "cfx/ui/Icons";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { Button } from "cfx/ui/Button/Button";

export const AppLayout: React.FC<{ children: React.ReactNode }> = ({ children }) => {


  return (
    <div className="app-layout">
      <div className='ungdung'>
        <Button
          theme="primary"
          size="large"
          icon={Icons.exit2}
          onClick={() => mpMenu.invokeNative('exit')}
        />
        <LinkButton
          to={'https://vngta.com'}
          icon={Icons.BsHousesZ}
          size="large"
          theme="transparent"
        />
        <LinkButton
          to={'https://vngta.com'}
          icon={Icons.FaDiscordZ}
          size="large"
          theme="transparent"
        />
       
      </div>
      {children}
    </div>
  );
};