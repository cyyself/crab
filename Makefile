.PHONY: clean

crab: crab.cpp
	g++ $< -O3 -lpthread -o $@

clean:
	rm crab || true