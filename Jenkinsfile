node('node') {

    currentBuild.result = "SUCCESS"

    try {

       stage 'Checkout'

            checkout scm

       stage 'Build'

            sh 'make'

       stage 'Deploy'

            echo 'Push to Repo'
            sh 'cp main /tmp/'

       stage 'Cleanup'

            sh 'make clean'

            mail body: 'project build successful',
                        from: 'xxxx@yyyyy.com',
                        replyTo: 'xxxx@yyyy.com',
                        subject: 'project build successful',
                        to: 'yyyyy@yyyy.com'

        }


    catch (err) {

        currentBuild.result = "FAILURE"

            mail body: "project build error is here: ${env.BUILD_URL}" ,
            from: 'xxxx@yyyy.com',
            replyTo: 'yyyy@yyyy.com',
            subject: 'project build failed',
            to: 'zzzz@yyyyy.com'

        throw err
    }

}
