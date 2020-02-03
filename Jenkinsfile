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

        deploy(projectName, envTagName)
    }
}

def deploy(project, environment) {

    def recreateContainerScriptUser = 'root'
    def recreateContainerScriptHost = 'puppet.devpons.io'

    docker.image('alpine:latest').inside('-u 0:0') {
        sh 'apk update && apk upgrade && apk add openssh-client'

        withCredentials([sshUserPrivateKey(credentialsId: 'deploy_key', keyFileVariable: 'KEYFILE_PATH')]) {
            sh 'mkdir -p ~/.ssh && cp -v $KEYFILE_PATH ~/.ssh/id_rsa && chmod 600 ~/.ssh/id_rsa'
        }

        sh 'ssh -oStrictHostKeyChecking=no -oBatchMode=yes -oPasswordAuthentication=no ' + recreateContainerScriptUser + '@' + recreateContainerScriptHost + ' "recreate-containers ' + project + ' ' + environment + '"'
    }
}
