import { Container, interfaces } from 'inversify';
import React, { createContext, useContext } from 'react';

export type ServicesContainerInitializer = (container: ServicesContainer) => void;

type SI<T> = interfaces.ServiceIdentifier<T> | ServiceIdentifier<T>;

export class ServicesContainer {
  protected container = new Container();

  protected cache = new Map<any, any>();

  constructor(initializer: ServicesContainerInitializer) {
    initializer(this);

    // eslint-disable-next-line no-unused-expressions
    this.container.isCurrentBound;
  }

  getContainer(): interfaces.Container {
    return this.container;
  }

  getAll<T>(id: SI<T>): T[] {
    if (!this.container.isBound(id)) {
      return [];
    }

    return this.container.getAll(id);
  }

  resolve<T>(ctor: interfaces.Newable<T>): T {
    return this.container.resolve(ctor);
  }

  get<T>(id: SI<T>): T {
    const cached = this.cache.get(id);

    if (cached) {
      return cached;
    }

    const resolved = this.container.get(id);

    this.cache.set(id, resolved);

    return resolved;
  }

  getOptional<T>(id: SI<T>): T | null {
    if (!this.container.isBound(id)) {
      return null;
    }

    return this.get(id);
  }

  registerConstant<T>(serviceId: SI<T>, value: T) {
    this.container.bind(serviceId).toConstantValue(value);
  }

  registerDynamic<T>(serviceId: SI<T>, value: interfaces.DynamicValue<T>) {
    this.container.bind(serviceId).toDynamicValue(value);
  }

  register<T>(impl: interfaces.Newable<T>) {
    this.container.bind(impl).toSelf().inSingletonScope();
  }

  registerImpl<T>(serviceId: SI<T>, impl: interfaces.Newable<T>) {
    if (!this.container.isBound(impl)) {
      this.register(impl);
    }

    this.container.bind(serviceId).toService(impl);
  }
}

export const ServicesContainerContext = createContext<ServicesContainer | void>(undefined);

export interface ServicesContainerContextProps {
  container: ServicesContainer;
}

function useServicesContainer(): ServicesContainer {
  const container = useContext(ServicesContainerContext);

  if (!container) {
    throw new Error('No ServicesContainer defined');
  }

  return container;
}

export function useServiceResolver(): <T>(id: interfaces.Newable<T>) => T {
  const container = useServicesContainer();

  return React.useCallback((id) => container.resolve(id), []);
}

export function useService<T>(id: SI<T>): T {
  return useServicesContainer().get(id);
}

export function useAllServices<T>(id: SI<T>): T[] {
  return useServicesContainer().getAll(id);
}

export function useServiceOptional<T>(id: SI<T>): T | null {
  return useServicesContainer().getOptional(id);
}

export type ServiceIdentifier<T> = (string | symbol) & {
  __this_property_does_not_exist_and_is_only_here_to_make_types_magic__: T;
};
export function defineService<T>(id: string | symbol): ServiceIdentifier<T> {
  return id as any;
}
