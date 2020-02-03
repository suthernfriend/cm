node {

    def scmVars = checkout scm
    def credentialsName = 'nexus_docker'
    def envTagName = params.TARGET_ENV
    def useDockerCache = params.USE_DOCKER_CACHE
    def registryUrl = 'https://nexus.services.devpons.io'
    def projectName = 'cm'

    def builtImage = null

    stage("Build Container") {
        if (useDockerCache) {
            builtImage = docker.build(projectName + ':' + scmVars.GIT_COMMIT)
        } else {
            builtImage = docker.build(projectName + ':' + scmVars.GIT_COMMIT, "--pull --no-cache .")
        }
        builtImage.tag(envTagName)
    }

    stage("Deploy") {
        docker.withRegistry(registryUrl, credentialsName) {
            builtImage.push(envTagName)
            builtImage.push()
        }
    }
}
