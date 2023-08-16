import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import s from './LoginLeftSide.module.scss';

export const LoginLeftSide = observer(function LoginLeftSide() {
    return (
        <Flex vertical gap="xlarge" className={s["left-side"]}>

        </Flex>
    );
  });
  