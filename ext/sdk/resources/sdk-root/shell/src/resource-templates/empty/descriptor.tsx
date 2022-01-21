import { BsDiamond } from "react-icons/bs";
import { ResourceTemplateDescriptor } from "../types";

export default {
  id: 'empty',
  icon: <div style={{ padding: '4px' }}><BsDiamond style={{ width: '100%', height: '100%' }} /></div>,
  title: 'No template',
  description: 'Empty resource',
} as ResourceTemplateDescriptor;
