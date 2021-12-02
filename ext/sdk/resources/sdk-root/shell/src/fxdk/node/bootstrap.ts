import 'reflect-metadata';
import 'backend/logger/logger-init';
import { ConfigService } from 'backend/config-service';
import { ContainerAccess, getContainer } from 'backend/container-access';

console.log('FxDK Shell Backend is now loading');

(Error as any).prepareStackTrace = (error: Error) => error.stack?.toString();

process.on('uncaughtException', (error) => {
  console.log('UNHANDLED EXCEPTION', error);
  process.exit(-1);
});

process.on('unhandledRejection', (error) => {
  console.log('UNHANDLED REJECTION', error);
  process.exit(-1);
});

const configService = new ConfigService();
getContainer().bind(ConfigService).toConstantValue(configService);
getContainer().bind(ContainerAccess).toConstantValue(new ContainerAccess(getContainer()));
