dados <- read.csv("results.csv")

plot(1:nrow(dados), dados$list_comparisons,
     type="l", col="red",
     xlab="Sorteio", ylab="Comparações",
     main="Lista vs BST")

lines(1:nrow(dados), dados$bst_comparisons, col="blue")

legend("topright",
       legend=c("Lista", "BST"),
       col=c("red", "blue"), lty=1)
