filename=$1
threshold=70
cleaner1="training-corpus.txt"
cleaner2="testing-corpus.txt"
cleaner3="clean.txt"
cleanernum=5

$(./split_corpus.sh "$threshold" "$filename")
$(./corpus_cleaner.sh "$cleaner1" "$cleaner3" "$cleanernum")
$(./corpus_cleaner.sh "$cleaner2" "$cleaner3" "$cleanernum")

# $(./file_checker.sh "$cleaner1")
# $(./file_checker.sh "$cleaner2")