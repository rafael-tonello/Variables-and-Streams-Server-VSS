#build tests
cd ../tests
make all

#checks if make of tests ware runned with sucess
if [ $? -eq 0 ]
then

    cd build

    #run tests
    ./tests

    #check if tests are runned with sucess
    if [ $? -eq 0 ]
    then
        #compile the main program
        cd ../../
        make all

        #checks if mains program was compiled with sucess
        if [ $? -eq 0 ]
        then
            #run git

            #access orangepi

            #make a copyt of current version

            #git pull on copy

            #compile main program (on copy)

            #run the compiled program

            #test the compiled program

            #if sucess, kill all process and replace the original folder by the copy

            #start the new version

            ffplay -autoexit -nodisp ./deploy/resources/sino_1.ogg &
            notify-send -a "Homeaut deplay tool" -t 10000 "Deploy sucess" | true
        else
            ffplay -autoexit -nodisp ../deploy/resources/error_2.ogg &
            notify-send -a "Homeaut deplay tool" -t 10000 "Error while run make on the main program" | true
        fi
    else
        ffplay -autoexit -nodisp ../../deploy/resources/error_2.ogg &
        notify-send -a "Homeaut deplay tool" -t 10000 "Error runing tests" | true
    fi
else
    ffplay -autoexit -nodisp ../deploy/resources/error_2.ogg &
    notify-send -a "Homeaut deplay tool" -t 10000 "Error while run make on tests" | true
fi