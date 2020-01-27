# cm - Container Daemon Manager
cm is a statically linked supervisord alternative designed specially for containerized legacy applications.

## About

CM is for you when you are in a situation like this:

- You sit on a 20 year old software project which, in it's current form, doesn't scale
- Management heard several buzzwords in the news and wants to join the "devops" cult
- You understood the principles of the [12-Factor-App](https://12factor.net/) and want to apply them to your application
- Your application depends on some services, like cronjobs or webproxies, but you really want to encapsulate them in a stateless blob.

Normally you would go with `supervisord` or `runit` in this case. cm is another alternative which is designed
exactly for this purpose and is configurable via a simple yaml file or even via environment variables. (WIP)

## Build

cm depends on yaml-cpp's static library, which you usually need to build yourself. However, since the main purpose
is using cm in a container you can simply use the prebuilt image on [Docker-Hub](https://hub.docker.com/r/suthernfriend/cm)

The image is based on Alpine, however since it's statically linked you can easily use it with any other distro.

```dockerfile

FROM suthernfriend/cm:latest as cm

FROM debian:stable

COPY --from=cm /usr/bin/cm /usr/bin/cm

COPY ./my-awesome-application /app
COPY ./cm.yaml /etc/cm.yaml

ENTRYPOINT [ '/usr/bin/cm', '-j', '/etc/cm.yaml' ]

```

## Configuration

cm's configuration file works as follows:

```yaml
# version is always required. currently there is only version 1
version: 1

# time after which processes will be sent SIGKILL after soft term signal
kill-delay: 5000

# a list with supervised applications to start
apps:

  # name of application
  nginx: 
    # the path in working directory for the process. required
    context: / 

    # exectable and arguments. required
    exec: nginx -g 'daemon off;'
    
    # should all other apps also be stopped when this process exits? default: true
    fail-on-exit: true
    
    # should all other apps also be stopped when this process exits with non-zero status code? default: true
    fail-on-nonzero-exit: true 
    
    # signal to send to the process to stop
    term-signal: SIGTERM

    # additional environment variables to only give to this process
    # parent environment is also passed to the apps
    env:
      VARIABLE_1: "value"
    
  java-app:
    context: /app
    exec: java -jar ./build/app.jar
    term-signal: SIGINT

  # ...
```
